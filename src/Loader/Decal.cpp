// 0x47a200: interpret the RLDecal fields and attach its texture to the current frame.
#include <windows.h>
#include <d3drm.h>
#include <string.h>

struct QvFieldEntry { const char **name; int offset; };
struct QvEntryList2 {
    QvFieldEntry **items; int count, capacity;
    QvFieldEntry *Get(int index);                  // 0x480810
    int OffsetOf(int node, int index);            // 0x480820
};
class QvRc {
public:
    virtual ~QvRc();
    int pad4, pad8, padc;
    void *name;
    int refCount;
    void Ref(); void Unref();
    void *GetName();                              // 0x47e780
};
struct DecalNode : QvRc {
    char *type;
    QvEntryList2 *fields;                         // +0x1c
};
struct DecalField {
    char pad[0xc];
    union { const char *text; int integer; float vector[3]; } value;
};
void WrlLog(const char *format, ...);              // 0x47a7b0
typedef IDirect3DRMTexture *(__cdecl *DecalTextureLoader)(const char *);

struct DecalLoader {
    void *vptr;
    int enabled;                                 // +0x04
    char pad[0x208 - 8];
    char path[256];                               // +0x208
    char pad308[8];
    DecalTextureLoader loadTexture;               // +0x310
    void *result;
    IDirect3DRMFrame *frame;                      // +0x318
    void LoadDecal(DecalNode *node, void *unused);
};

void DecalLoader::LoadDecal(DecalNode *node, void *unused)
{
    const char *filename = "";
    float originX=0.0f, originY=0.0f;
    float width=1.0f, height=1.0f;
    int colors=8, transparent=0;
    float red=0.0f, green=0.0f, blue=0.0f;
    int count=node->fields->count;
    for (int i=0; i<count; ++i) {
        const char *name=*node->fields->Get(i)->name;
        DecalField *field=(DecalField *)node->fields->OffsetOf((int)node, i);
        if (strcmp(name,"filename")==0) filename=field->value.text;
        else if (strcmp(name,"origin")==0) {
            originX=field->value.vector[0]; originY=field->value.vector[1];
        } else if (strcmp(name,"size")==0) {
            width=field->value.vector[0]; height=field->value.vector[1];
        } else if (strcmp(name,"colors")==0) colors=field->value.integer;
        else if (strcmp(name,"useTransparency")==0) transparent=field->value.integer;
        else if (strcmp(name,"transparentColor")==0) {
            red=field->value.vector[0]; green=field->value.vector[1]; blue=field->value.vector[2];
        } else WrlLog("Unrecognized '%s' field in RLDecal in %s",name,path);
    }
    IDirect3DRMTexture *texture=loadTexture(filename);
    if (!texture) {
        WrlLog("Can't load %s for RLDecal in %s",filename,path);
        enabled=0;
        return;
    }
    texture->SetName(**(const char ***)node->GetName());
    texture->SetColors(colors);
    D3DRMIMAGE *image=texture->GetImage();
    if (!image) {
        WrlLog("Can't get image data from decal texture in %s",path);
        enabled=0;
        return;
    }
    texture->SetDecalOrigin((long)(image->width*originX),(long)(image->height*originY));
    texture->SetDecalSize(width,height);
    texture->SetDecalTransparency(transparent);
    unsigned color=((unsigned)(int)(red*255.0f) | 0xffffff00U) << 16;
    color |= (unsigned)(int)(green*255.0f) << 8;
    color |= (unsigned)(int)(blue*255.0f);
    texture->SetDecalTransparentColor(color);
    if (frame->AddVisual(texture)<0) {
        WrlLog("Can't add RLDecal to Frame in %s",path);
        enabled=0;
    }
    texture->Release();
}
