// SQZ.EXE 0x4734e0: reset base positions, add weighted deltas and upload mesh groups.
#include <windows.h>
#include <d3drm.h>
#include "SqgModel.h"

struct MorphVector {
    float x, y, z;
    MorphVector(const MorphVector &v) : x(v.x), y(v.y), z(v.z) {}
    ~MorphVector();                                  // 0x401660
    MorphVector operator*(float w) const;            // 0x401b60
    MorphVector operator+(const MorphVector &v) const; // 0x403070
};
struct MorphVertex { MorphVector position; float other[6]; }; // 0x24 bytes
struct MorphDelta { int vertex; MorphVector offset; };       // 0x10 bytes
struct MorphDeltaList { int count; MorphDelta *deltas; };
struct MorphGroup {
    int color, unused4, unused8, texture;
    int firstVertex, vertexCount, faceCount, indexStart, indexCount;
};
struct PZMorphMesh {
    int *groupIds;
    MorphVertex *vertices;
    IDirect3DRMMesh *mesh;
};

void SqgModel::Morph(PZMorphMesh *instance, int count, const float *weights)
{
    for (int v = 0; v < vertexCount; v++)
        instance->vertices[v].position = ((MorphVertex *)vertices)[v].position;
    for (int k = 0; k < count; k++) {
        float w = weights[k];
        // The original tests the magnitude bits, preserving NaNs and skipping both signed zeros.
        if ((*(unsigned *)&w & 0x7fffffffU) != 0) {
            MorphDeltaList *list = &((MorphDeltaList *)deltaLists)[k];
            for (int d = 0; d < list->count; d++) {
                MorphDelta *delta = &list->deltas[d];
                MorphVector offset = delta->offset;
                instance->vertices[delta->vertex].position =
                    instance->vertices[delta->vertex].position + offset * w;
            }
        }
    }
    for (int g = 0; g < groupCount; g++) {
        MorphGroup *group = &((MorphGroup *)groups)[g];
        if (group->vertexCount > 0)
            instance->mesh->SetVertices(instance->groupIds[g], 0, group->vertexCount,
                (D3DRMVERTEX *)&instance->vertices[group->firstVertex]);
    }
}
