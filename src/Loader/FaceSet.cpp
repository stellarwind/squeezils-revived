// 0x479b80: build a MeshBuilder from a VRML IndexedFaceSet and its current state nodes, and add it to the current frame.
#include <windows.h>
#include <d3drm.h>

// QvLib multiple/single-value fields as they sit inside a node (SIZE 0x18 each).
struct QvMFLong  { void *vptr; int flags, owner; int num; int maxNum; long *values; };          // num 0x0c, values 0x14
struct QvMFFloat { void *vptr; int flags, owner; int num; int maxNum; float *values; };
struct QvMFVec3f { void *vptr; int flags, owner; int num; int maxNum; float (*values)[3]; };
struct QvMFVec2f { void *vptr; int flags, owner; int num; int maxNum; float (*values)[2]; };
struct QvSFEnum  { void *vptr; int flags, owner; int value; int pad10, pad14; };
struct QvSFString { void *vptr; int flags, owner; char *string; int pad10, pad14; };

class QvNode {
public:
    virtual ~QvNode();
    int pad4, pad8, padc;
    void *name;                                   // 0x10
    int refCount;                                 // 0x14
    void *GetName();                              // 0x47e780
};
struct QvIndexedFaceSet : QvNode {
    QvMFLong coordIndex;                          // 0x18
    QvMFLong materialIndex;                       // 0x30
    QvMFLong normalIndex;                         // 0x48
    QvMFLong textureCoordIndex;                   // 0x60
};
struct QvCoordinate3 : QvNode { QvMFVec3f point; };          // 0x18
struct QvNormal : QvNode { QvMFVec3f vector; };              // 0x18
struct QvTextureCoordinate2 : QvNode { QvMFVec2f point; };   // 0x18
struct QvTexture2 : QvNode { QvSFString filename; };         // 0x18
struct QvMaterialBinding : QvNode { QvSFEnum value; };       // 0x18
struct QvMaterial : QvNode {
    QvMFVec3f ambientColor;                       // 0x18
    QvMFVec3f diffuseColor;                       // 0x30
    QvMFVec3f specularColor;                      // 0x48
    QvMFVec3f emissiveColor;                      // 0x60
    QvMFFloat shininess;                          // 0x78
    QvMFFloat transparency;                       // 0x90
};
enum { PER_FACE_INDEXED = 5 };

// SIZE 0x8: a face colour plus a COM object (Misc18/ComHolder.cpp; 0x47a100 is its destructor).
struct ComHolder {
    unsigned color;                               // 0x0
    IUnknown *obj;                                // 0x4
    ComHolder();                                  // 0x47a0f0
    ~ComHolder();                                 // 0x47a100
};

void WrlLog(const char *format, ...);              // 0x47a7b0
typedef IDirect3DRMTexture *(__cdecl *DecalTextureLoader)(const char *);
extern int gFlipHandedness;                        // 0x4c39f0: mirror x and the face winding

// The WRL loader object (same class as in Decal.cpp).
struct DecalLoader {
    void *vptr;
    int enabled;                                  // 0x04
    char pad[0x208 - 8];
    char path[256];                               // 0x208
    int pad308;
    IDirect3DRM *d3drm;                           // 0x30c
    DecalTextureLoader loadTexture;               // 0x310
    void *result;                                 // 0x314
    IDirect3DRMFrame *frame;                      // 0x318
    void LoadFaceSet(QvIndexedFaceSet *faceSet, QvCoordinate3 *coords, QvNormal *normals, QvNode *normalBinding,
                     QvMaterial *material, QvMaterialBinding *materialBinding, QvTexture2 *texture,
                     QvTextureCoordinate2 *texCoords, QvNode *shapeHints, int counterClockwise, QvNode *unused);
};

void DecalLoader::LoadFaceSet(QvIndexedFaceSet *faceSet, QvCoordinate3 *coords, QvNormal *normals, QvNode *normalBinding,
                              QvMaterial *material, QvMaterialBinding *materialBinding, QvTexture2 *texture,
                              QvTextureCoordinate2 *texCoords, QvNode *shapeHints, int counterClockwise, QvNode *unused)
{
    int faceCount = 0;
    int i;
    for (i = 0; i < faceSet->coordIndex.num; i++)
        if (faceSet->coordIndex.values[i] == -1)
            faceCount++;

    IDirect3DRMMeshBuilder *builder;
    if (d3drm->CreateMeshBuilder(&builder) < 0) {
        WrlLog("Can't create Mesh Builder for IndexedFaceSet in %s", path);
        enabled = 0;
        return;
    }
    builder->SetName(**(const char ***)faceSet->GetName());
    if (builder->ReserveSpace(coords->point.num, normals ? normals->vector.num : coords->point.num, faceCount) < 0) {
        WrlLog("Can't allocate %d coords, %d normals and %d faces for Mesh in %s",
               coords->point.num, normals ? normals->vector.num : coords->point.num, faceCount, path);
        builder->Release();
        enabled = 0;
        return;
    }

    float scale = gFlipHandedness ? -1.0f : 1.0f;
    for (i = 0; i < coords->point.num; i++)
        builder->AddVertex(coords->point.values[i][0] * scale, coords->point.values[i][1], coords->point.values[i][2]);
    if (normals) {
        for (i = 0; i < normals->vector.num; i++)
            builder->AddNormal(normals->vector.values[i][0] * scale, normals->vector.values[i][1], normals->vector.values[i][2]);
    } else {
        for (i = 0; i < coords->point.num; i++)
            builder->AddNormal(0.0f, 0.0f, 1.0f);
    }

    IDirect3DRMTexture *tex = 0;
    if (texture)
        tex = loadTexture(texture->filename.string);
    if (texCoords)
        for (i = 0; i < texCoords->point.num; i++)
            builder->SetTextureCoordinates(i, texCoords->point.values[i][0], texCoords->point.values[i][1]);

    ComHolder *colors = 0;
    if (material) {
        int diffuseCount = material->diffuseColor.num;
        int specularCount = material->specularColor.num;
        int transparencyCount = material->transparency.num;
        int shininessCount = material->shininess.num;
        int count = specularCount > diffuseCount ? specularCount : diffuseCount;
        int other = shininessCount > transparencyCount ? shininessCount : transparencyCount;
        if (count <= other)
            count = other;
        colors = new ComHolder[count];
        for (i = 0; i < count; i++) {
            int d = i < diffuseCount ? i : diffuseCount - 1;
            int t = i < transparencyCount ? i : transparencyCount - 1;
            float *diffuse = material->diffuseColor.values[d];
            colors[i].color = RGBA_MAKE((int)(diffuse[0] * 255.0f), (int)(diffuse[1] * 255.0f), (int)(diffuse[2] * 255.0f),
                                        (int)((1.0f - material->transparency.values[t]) * 255.0f));
            colors[i].obj = 0;
        }
    }

    int start = 0;
    for (int f = 0; f < faceCount; f++) {
        IDirect3DRMFace *face;
        if (builder->CreateFace(&face) < 0) {
            WrlLog("Can't create face %d", f);
            continue;
        }
        int n = 0;
        for (long *p = &faceSet->coordIndex.values[start]; *p != -1; p++)
            n++;
        int reverse = (!gFlipHandedness && !counterClockwise) || (gFlipHandedness && counterClockwise);
        for (int j = 0; j < n; j++) {
            long vertex, normal;
            if (reverse) {
                vertex = faceSet->coordIndex.values[start + n - 1 - j];
                normal = faceSet->normalIndex.num == faceSet->coordIndex.num ? faceSet->normalIndex.values[start + n - 1 - j] : vertex;
            } else {
                vertex = faceSet->coordIndex.values[start + j];
                normal = faceSet->normalIndex.num == faceSet->coordIndex.num ? faceSet->normalIndex.values[start + j] : vertex;
            }
            face->AddVertexAndNormalIndexed(vertex, normal);
        }
        start += n + 1;
        if (colors) {
            int m = 0;
            if (materialBinding && materialBinding->value.value == PER_FACE_INDEXED)
                m = faceSet->materialIndex.values[f];
            face->SetColor(colors[m].color);
        }
        face->Release();
        face = 0;
    }
    delete [] colors;

    if (tex)
        builder->SetTexture(tex);
    if (!normals)
        builder->GenerateNormals();
    if (frame->AddVisual(builder) < 0) {
        WrlLog("Can't add MeshBuilder to Frame for IndexedFaceSet in %s", path);
        enabled = 0;
    }
    builder->Release();
}
