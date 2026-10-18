// Canonical observer of the SGI read path. No RTTI and no traverse() calls.
#include <QvDB.h>
#include <QvInput.h>
#include <QvNode.h>
#include <QvChildList.h>
#include <QvNodes.h>
#include <QvUnknownNode.h>
#include <QvFields.h>
#include <QvFieldData.h>
#include <locale.h>

struct NodeType { void *vt; const char *name; int group; };
struct FieldType { void *vt; const char *name; };
static NodeType nt[40]; static int nn;
static FieldType ft[20]; static int nf;
static QvNode *seen[100000]; static int nseen;

static void *vtable(const void *p) { void *v; memcpy(&v, p, sizeof v); return v; }
static void nodeType(QvNode *p, const char *name, int group)
{ nt[nn].vt=vtable(p); nt[nn].name=name; nt[nn++].group=group; }
static void fieldType(QvField *p, const char *name)
{ ft[nf].vt=vtable(p); ft[nf++].name=name; }
static void initTypes()
{
#define N(c,g) nodeType(new Qv##c, #c, g)
    N(AsciiText,0); N(Cone,0); N(Coordinate3,0); N(Cube,0); N(Cylinder,0);
    N(DirectionalLight,0); N(FontStyle,0); N(Group,1); N(IndexedFaceSet,0);
    N(IndexedLineSet,0); N(Info,0); N(LOD,1); N(Material,0); N(MaterialBinding,0);
    N(MatrixTransform,0); N(Normal,0); N(NormalBinding,0); N(OrthographicCamera,0);
    N(PerspectiveCamera,0); N(PointLight,0); N(PointSet,0); N(Rotation,0);
    N(Scale,0); N(Separator,1); N(ShapeHints,0); N(Sphere,0); N(SpotLight,0);
    N(Switch,1); N(Texture2,0); N(Texture2Transform,0); N(TextureCoordinate2,0);
    N(Transform,0); N(TransformSeparator,1); N(Translation,0); N(WWWAnchor,1);
    N(WWWInline,1); N(UnknownNode,1);
#undef N
#define F(c) fieldType(new Qv##c, #c)
    F(SFBitMask); F(SFBool); F(SFColor); F(SFEnum); F(SFFloat); F(SFImage);
    F(SFLong); F(SFMatrix); F(SFRotation); F(SFString); F(SFVec2f); F(SFVec3f);
    F(MFColor); F(MFFloat); F(MFLong); F(MFString); F(MFVec2f); F(MFVec3f);
#undef F
}
static void quoted(const char *s)
{
    putchar('"');
    for (const unsigned char *p=(const unsigned char *)s; *p; ++p) {
        if (*p=='"' || *p=='\\') { putchar('\\'); putchar(*p); }
        else if (*p=='\n') printf("\\n");
        else if (*p=='\r') printf("\\r");
        else if (*p=='\t') printf("\\t");
        else if (*p<32 || *p>=127) printf("\\x%02x",(unsigned)*p);
        else putchar(*p);
    }
    putchar('"');
}
static void number(float v)
{
    char buf[100]; unsigned long bits; memcpy(&bits,&v,4);
    sprintf(buf,"%.9g",(double)v);
    if(getenv("QV_FLOAT_BITS")) { printf("%s{0x%08lx}",buf,bits); return; }
    if(!getenv("QV_RAW_FLOATS")) {
        // VC4's CRT hides negative zero and uses three-digit exponents.
        // Preserve the IEEE sign bit and use a minimum two-digit exponent.
        if(v==0 && (bits & 0x80000000UL)) strcpy(buf,"-0");
        char *e=strchr(buf,'e');
        if(e) while(e[2]=='0' && strlen(e+2)>2) memmove(e+2,e+3,strlen(e+3)+1);
    }
    printf("%s",buf);
}
static void floats(const float *v, int n)
{ putchar('['); for(int i=0;i<n;++i) { if(i) putchar(' '); number(v[i]); } putchar(']'); }
static const char *fieldName(QvField *f)
{
    for(int i=0;i<nf;++i) if(ft[i].vt==vtable(f)) return ft[i].name;
    fprintf(stderr,"DUMPER ERROR: unrecognized field vtable\n"); exit(3); return "";
}
static void value(QvField *f, const char *t)
{
#define IS(s) (!strcmp(t,#s))
    if(IS(SFBitMask) || IS(SFEnum)) printf("%d",((QvSFEnum *)f)->value);
    else if(IS(SFBool)) printf("%d",((QvSFBool *)f)->value);
    else if(IS(SFLong)) printf("%ld",((QvSFLong *)f)->value);
    else if(IS(SFFloat)) number(((QvSFFloat *)f)->value);
    else if(IS(SFColor)) floats(((QvSFColor *)f)->value,3);
    else if(IS(SFVec2f)) floats(((QvSFVec2f *)f)->value,2);
    else if(IS(SFVec3f)) floats(((QvSFVec3f *)f)->value,3);
    else if(IS(SFMatrix)) floats(&((QvSFMatrix *)f)->value[0][0],16);
    else if(IS(SFRotation)) {
        QvSFRotation *r=(QvSFRotation *)f; floats(r->axis,3); printf(" angle="); number(r->angle);
    }
    else if(IS(SFString)) quoted(((QvSFString *)f)->value.getString());
    else if(IS(SFImage)) {
        QvSFImage *im=(QvSFImage *)f;
        printf("%d %d %d [",(int)im->size[0],(int)im->size[1],im->numComponents);
        int n=im->size[0]*im->size[1]*im->numComponents;
        for(int i=0;i<n;++i) { if(i) putchar(' '); printf("%02x",(unsigned)im->bytes[i]); }
        putchar(']');
    }
    else {
        QvMField *m=(QvMField *)f;
        printf("num=%d ",m->num);
        if(IS(MFFloat)) floats(((QvMFFloat *)f)->values,m->num);
        else if(IS(MFColor)) floats(((QvMFColor *)f)->values,m->num*3);
        else if(IS(MFVec2f)) floats(((QvMFVec2f *)f)->values,m->num*2);
        else if(IS(MFVec3f)) floats(((QvMFVec3f *)f)->values,m->num*3);
        else if(IS(MFLong)) {
            QvMFLong *l=(QvMFLong *)f; putchar('[');
            for(int i=0;i<m->num;++i) { if(i) putchar(' '); printf("%ld",l->values[i]); } putchar(']');
        }
        else if(IS(MFString)) {
            QvMFString *s=(QvMFString *)f; putchar('[');
            for(int i=0;i<m->num;++i) { if(i) putchar(' '); quoted(s->values[i].getString()); } putchar(']');
        }
        else { fprintf(stderr,"DUMPER ERROR: unsupported field\n"); exit(3); }
    }
#undef IS
}
static void indent(int depth) { for(int i=0;i<depth;++i) printf("  "); }
static void dump(QvNode *node,int depth)
{
    indent(depth);
    for(int i=0;i<nseen;++i) if(seen[i]==node) { printf("USE id=%d\n",i+1); return; }
    if(nseen==100000) { fprintf(stderr,"DUMPER ERROR: node limit\n"); exit(3); }
    seen[nseen++]=node;
    int type=-1;
    for(int j=0;j<nn;++j) if(nt[j].vt==vtable(node)) { type=j; break; }
    if(type<0) { fprintf(stderr,"DUMPER ERROR: unrecognized node\n"); exit(3); }
    printf("NODE id=%d class=",nseen);
    if(!strcmp(nt[type].name,"UnknownNode")) quoted(((QvUnknownNode *)node)->className);
    else quoted(nt[type].name);
    printf(" DEF="); quoted(node->getName().getString()); putchar('\n');
    // The stock UnknownNode accessor returns shared static metadata, which may
    // belong to a later unknown class in the same tree. Use its public per-node
    // schema; this observes the parsed tree without changing parser behavior.
    QvFieldData *data=!strcmp(nt[type].name,"UnknownNode") ?
        ((QvUnknownNode *)node)->instanceFieldData : node->getFieldData();
    for(int k=0;k<data->getNumFields();++k) {
        QvField *f=data->getField(node,k); const char *t=fieldName(f);
        indent(depth+1); printf("FIELD "); quoted(data->getFieldName(k).getString());
        printf(" type=%s default=%d value=",t,f->isDefault()); value(f,t); putchar('\n');
    }
    if(nt[type].group) {
        QvGroup *g=(QvGroup *)node;
        for(int c=0;c<g->getNumChildren();++c) dump(g->getChild(c),depth+1);
    }
}
int main(int argc,char **argv)
{
    if(argc!=2) return 2;
    setlocale(LC_ALL,"C"); setvbuf(stdout,0,_IONBF,0); setvbuf(stderr,0,_IONBF,0);
    FILE *file=fopen(argv[1],"r"); // Translate CRLF, as the SGI reader expects.
    if(!file) { printf("OPEN_ERROR\nOVERALL return=0\n"); return 2; }
    QvDB::init(); initTypes();
    QvInput in; in.setFilePointer(file);
    QvNode *roots[10000]; int nroots=0, reads=0, overall=1;
    for(;;) {
        QvNode *root=0; int ok=QvDB::read(&in,root);
        printf("READ %d return=%d\n",++reads,ok);
        if(!ok) { overall=0; break; }
        if(!root) break;
        if(nroots==10000) { fprintf(stderr,"DUMPER ERROR: root limit\n"); return 3; }
        root->ref(); roots[nroots++]=root; dump(root,0);
    }
    printf("OVERALL return=%d roots=%d unique_nodes=%d\n",overall,nroots,nseen);
    for(int i=0;i<nroots;++i) roots[i]->unref();
    fclose(file); return overall ? 0 : 1;
}
