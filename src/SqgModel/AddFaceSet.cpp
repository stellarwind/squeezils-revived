// 0x471b20: append a VRML IndexedFaceSet and its current state nodes to the SqgModel being loaded.
// The SQG twin of DecalLoader::LoadFaceSet (Loader/FaceSet.cpp, 0x479b80): same callback slot, same arguments,
// but it fills the model's flat arrays instead of a D3DRM MeshBuilder.
#include <windows.h>
#include <d3drm.h>
#include <string.h>
#include "SqgModel/SqgModel.h"

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
enum { OVERALL = 1, PER_FACE_INDEXED = 5 };

// SIZE 0x24: one vertex of an SqgModel.
struct SqgVertex {
    float x, y, z;                                // 0x00
    float nx, ny, nz;                             // 0x0c
    float u, v;                                   // 0x18
    D3DCOLOR color;                               // 0x20
};
// SIZE 0x24: one material group of an SqgModel. Its index stream is, per face, a vertex count then that many vertex indices.
struct SqgGroup {
    D3DCOLOR color;                               // 0x00
    int pad04, pad08;                             // 0x04, 0x08
    int texture;                                  // 0x0c: index into textureNames, or -1
    int firstVertex;                              // 0x10
    int vertexCount;                              // 0x14
    int faceCount;                                // 0x18
    int firstIndex;                               // 0x1c
    int indexCount;                               // 0x20
};
#define GROUPS   ((SqgGroup *)model->groups)
#define VERTICES ((SqgVertex *)model->vertices)

extern void *(*Alloc)(size_t n);                  // 0x4c0f78
extern void *(*Realloc)(void *p, size_t n);       // 0x4c0f7c
extern void (*Free)(void *p);                     // 0x4c0f80

// The WRL loader object for SqgModel::LoadWrl (LoadWrl.cpp; ctor 0x471ab0, vtable 0x4a1040).
struct WrlLoadContext {
    virtual void AddFaceSet(QvIndexedFaceSet *faceSet, QvCoordinate3 *coords, QvNormal *normals, QvNode *normalBinding,
                            QvMaterial *material, QvMaterialBinding *materialBinding, QvTexture2 *texture,
                            QvTextureCoordinate2 *texCoords, QvNode *shapeHints, int counterClockwise, QvNode *unused); // vtable+0x00
    int ok;                                       // 0x04
    char state[0x304];
    SqgModel *model;                              // 0x30c
    int noShare;                                  // 0x310: never share vertices between faces
};

void WrlLoadContext::AddFaceSet(QvIndexedFaceSet *faceSet, QvCoordinate3 *coords, QvNormal *normals, QvNode *normalBinding,
                                QvMaterial *material, QvMaterialBinding *materialBinding, QvTexture2 *texture,
                                QvTextureCoordinate2 *texCoords, QvNode *shapeHints, int counterClockwise, QvNode *unused)
{
    int i, g, f, j, k;
    int textureIndex = -1;
    if (texture) {
        if (!model->textureNames)
            model->textureNames = (char **)Alloc(4);
        else
            model->textureNames = (char **)Realloc(model->textureNames, (model->textureCount + 1) * 4);
        const char *name = texture->filename.string;
        model->textureNames[model->textureCount] = (char *)Alloc(strlen(name) + 1);
        strcpy(model->textureNames[model->textureCount], name);
        textureIndex = model->textureCount++;
    }

    // One group per material entry, or a single one.
    int groupBase = model->groupCount;
    int groupsAdded;
    if (material) {
        int diffuseCount = material->diffuseColor.num;
        int specularCount = material->specularColor.num;
        int transparencyCount = material->transparency.num;
        int shininessCount = material->shininess.num;
        int count = diffuseCount > specularCount ? diffuseCount : specularCount;
        int other = transparencyCount > shininessCount ? transparencyCount : shininessCount;
        groupsAdded = count > other ? count : other;
    } else
        groupsAdded = 1;
    model->groupCount += groupsAdded;
    if (!model->groups)
        model->groups = Alloc(model->groupCount * 0x24);
    else
        model->groups = Realloc(model->groups, model->groupCount * 0x24);

    D3DCOLOR color = 0;
    if (material)
        color = D3DRMCreateColorRGBA(material->diffuseColor.values[0][0], material->diffuseColor.values[0][1],
                                     material->diffuseColor.values[0][2], 1.0f - material->transparency.values[0]);
    GROUPS[groupBase].color = color;
    GROUPS[groupBase].pad04 = 0;
    GROUPS[groupBase].pad08 = 0;
    GROUPS[groupBase].texture = textureIndex;
    if (material) {
        int diffuseCount = material->diffuseColor.num;
        int transparencyCount = material->transparency.num;
        for (i = 0; i < groupsAdded; i++) {
            int d = i < diffuseCount ? i : diffuseCount - 1;
            int t = i < transparencyCount ? i : transparencyCount - 1;
            float *diffuse = material->diffuseColor.values[d];
            GROUPS[groupBase + i].color = D3DRMCreateColorRGBA(diffuse[0], diffuse[1], diffuse[2],
                                                               1.0f - material->transparency.values[t]);
            GROUPS[groupBase + i].pad04 = 0;
            GROUPS[groupBase + i].pad08 = 0;
            GROUPS[groupBase + i].texture = textureIndex;
        }
    }

    // Pass 1: copy the faces into the index stream, group by group; count the vertices that will be needed.
    int coordIndexCount = faceSet->coordIndex.num;
    int coordCount = coords->point.num;
    long *normalOf = (long *)Alloc(coordIndexCount * 4);   // per index-stream slot: its normal, -1 for a face's count
    int indexBase = model->indexCount;
    model->indexCount += coordIndexCount;
    if (!model->indices)
        model->indices = (int *)Alloc(coordIndexCount * 4);
    else
        model->indices = (int *)Realloc(model->indices, model->indexCount * 4);

    int faceCount = 0;
    for (i = 0; i < coordIndexCount; i++)
        if (faceSet->coordIndex.values[i] == -1)
            faceCount++;
    model->faceCount += faceCount;

    int hasNormalIndex = faceSet->normalIndex.num == coordIndexCount;
    int newVertices = hasNormalIndex ? coordIndexCount - faceCount : 0;
    // per coordinate: 0 unused, 1 used by this group, 2 used by an earlier group (needs a copy)
    int *vertexOf = (int *)Alloc(coordCount * 4);
    for (i = 0; i < coordCount; i++)
        vertexOf[i] = 0;

    int cur = indexBase;
    for (g = groupBase; g < groupBase + groupsAdded; g++) {
        int coordPos = 0;
        int groupFaces = 0;
        GROUPS[g].firstIndex = cur;
        for (f = 0; f < faceCount; f++) {
            int n = 0;
            for (long *p = &faceSet->coordIndex.values[coordPos]; *p != -1; p++)
                n++;
            long m = f < faceSet->materialIndex.num ? faceSet->materialIndex.values[f]
                                                    : faceSet->materialIndex.values[faceSet->materialIndex.num - 1];
            if (materialBinding && ((materialBinding->value.value == PER_FACE_INDEXED && g - groupBase != m) ||
                                    (materialBinding->value.value == OVERALL && g > groupBase))) {
                coordPos += n + 1;
                continue;
            }
            model->indices[cur] = n;
            normalOf[cur - indexBase] = -1;
            cur++;
            groupFaces++;
            for (j = 0; j < n; j++) {
                long vertex = counterClockwise ? faceSet->coordIndex.values[coordPos + j]
                                               : faceSet->coordIndex.values[coordPos + n - 1 - j];
                model->indices[cur + j] = vertex;
                if (hasNormalIndex)
                    normalOf[cur - indexBase + j] = faceSet->normalIndex.values[coordPos + j];
                else {
                    normalOf[cur - indexBase + j] = vertex;
                    if (noShare)
                        newVertices++;
                    else if (vertexOf[vertex] == 0 || vertexOf[vertex] == 2) {
                        vertexOf[vertex] = 1;
                        newVertices++;
                    }
                }
            }
            cur += n;
            coordPos += n + 1;
        }
        if (!hasNormalIndex)
            for (i = 0; i < coordCount; i++)
                if (vertexOf[i] == 1)
                    vertexOf[i] = 2;
        GROUPS[g].indexCount = cur - GROUPS[g].firstIndex;
        GROUPS[g].faceCount = groupFaces;
    }

    // Pass 2: create the vertices and point the index stream at them.
    int vertex = model->vertexCount;
    model->vertexCount += newVertices;
    if (!model->vertices)
        model->vertices = Alloc(newVertices * 0x24);
    else
        model->vertices = Realloc(model->vertices, model->vertexCount * 0x24);
    for (i = 0; i < coordCount; i++)
        vertexOf[i] = -1;

    for (g = groupBase; g < groupBase + groupsAdded; g++) {
        int left = 0;
        GROUPS[g].firstVertex = vertex;
        for (k = 0; k < GROUPS[g].indexCount; k++) {
            SqgGroup *group = &GROUPS[g];
            int slot = group->firstIndex + k;
            if (left == 0) {
                left = model->indices[slot];
                continue;
            }
            left--;
            int *p = &model->indices[slot];
            int c = *p;
            int shared = vertexOf[c];
            if (!noShare && !hasNormalIndex && shared != -1) {
                if (shared < group->firstVertex) {
                    VERTICES[vertex] = VERTICES[shared];
                    model->indices[slot] = vertex;
                    vertexOf[c] = vertex;
                    vertex++;
                } else
                    *p = shared;
                continue;
            }
            vertexOf[c] = vertex;
            model->indices[slot] = vertex;
            SqgVertex *v = &VERTICES[vertex];
            v->x = coords->point.values[c][0];
            v->y = coords->point.values[c][1];
            v->z = coords->point.values[c][2];
            long normal = normalOf[slot - indexBase];
            v->nx = normals->vector.values[normal][0];
            v->ny = normals->vector.values[normal][1];
            v->nz = normals->vector.values[normal][2];
            if (texCoords) {
                v->u = texCoords->point.values[c][0];
                v->v = texCoords->point.values[c][1];
            } else {
                v->v = 0;
                v->u = 0;
            }
            v->color = GROUPS[g].color;
            vertex++;
        }
        GROUPS[g].vertexCount = vertex - GROUPS[g].firstVertex;
    }

    Free(vertexOf);
    Free(normalOf);
}
