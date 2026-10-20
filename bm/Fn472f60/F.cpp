// 0x472f60: SqgModel::CreateMesh - one mesh group per model group; face indices shifted group-local around AddGroup.
#include <windows.h>
#include <d3drm.h>
#include <string.h>
struct SqgGroup {
    int color, f04, f08, textureIndex, firstVertex, vertexCount, faceCount, indexStart, indexCount;
};
typedef IDirect3DRMTexture *(*TextureLookupFn)(const char *name);
extern TextureLookupFn gTextureLookup;                       // 0x4c0f84 (via helper)
TextureLookupFn GetTextureLookup();
extern void *(*Alloc)(size_t n);                              // 0x4c0f78
struct Instance {
    int *groupIds;
    D3DRMVERTEX *vertices;
    IDirect3DRMMesh *mesh;
    Instance() { groupIds = 0; vertices = 0; mesh = 0; }
};
struct SqgModel {
    int textureCount;       // 0x00
    char **textureNames;    // 0x04
    int vertexCount;        // 0x08
    D3DRMVERTEX *vertices;  // 0x0c
    int faceCount;          // 0x10
    int indexCount;         // 0x14
    int *indices;           // 0x18
    int groupCount;         // 0x1c
    SqgGroup *groups;       // 0x20
    Instance *CreateMesh(IDirect3DRM *d3drm);
};
#define G ((SqgGroup *)((char *)groups + off))
Instance *SqgModel::CreateMesh(IDirect3DRM *d3drm)
{
    IDirect3DRMMesh *mesh;
    int *groupIds;
    int g, off, i, j, skip;
    D3DRMGROUPINDEX id;
    d3drm->CreateMesh(&mesh);
    mesh->SetName("PZMorph Mesh");
    groupIds = (int *)Alloc(groupCount * 4);
    g = 0;
    if (groupCount > 0) {
        off = 0;
        do {
            if (G->vertexCount != 0) {
                skip = 0;
                j = G->indexStart;
                for (i = 0; i < G->indexCount; i++, j++) {
                    if (skip == 0)
                        skip = indices[j];
                    else {
                        skip--;
                        indices[j] -= G->firstVertex;
                    }
                }
                mesh->AddGroup(G->vertexCount, G->faceCount, 0, (unsigned *)&indices[G->indexStart], &id);
                groupIds[g] = (int)id;
                mesh->SetVertices(id, 0, G->vertexCount, &vertices[G->firstVertex]);
                j = G->indexStart;
                for (i = 0; i < G->indexCount; i++, j++) {
                    if (skip == 0)
                        skip = indices[j];
                    else {
                        skip--;
                        indices[j] += G->firstVertex;
                    }
                }
                if (G->textureIndex != -1)
                    mesh->SetGroupColor(id, 0xffffffff);
                else
                    mesh->SetGroupColor(id, G->color);
                TextureLookupFn lookup = GetTextureLookup();
                if (G->textureIndex != -1 && lookup)
                    mesh->SetGroupTexture(id, lookup(textureNames[G->textureIndex]));
            }
            g++;
            off += 0x24;
        } while (g < groupCount);
    }
    D3DRMVERTEX *copy = (D3DRMVERTEX *)Alloc(vertexCount * sizeof(D3DRMVERTEX));
    memcpy(copy, vertices, vertexCount * sizeof(D3DRMVERTEX));
    Instance *inst = new Instance;
    inst->groupIds = groupIds;
    inst->vertices = copy;
    inst->mesh = mesh;
    return inst;
}
