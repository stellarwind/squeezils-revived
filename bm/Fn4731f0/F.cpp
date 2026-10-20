// 0x4731f0: SqgModel::BuildFrame - one mesh builder per group, filled vertex by vertex and face by face, added to a new frame.
#include <windows.h>
#include <d3drm.h>
struct SqgGroup {
    int color, f04, f08, textureIndex, firstVertex, vertexCount, faceCount, indexStart, indexCount;
};
typedef IDirect3DRMTexture *(*TextureLookupFn)(const char *name);
TextureLookupFn GetTextureLookup();                          // 0x4c0f84 holder (thiscall-free helper)
#define G ((SqgGroup *)((char *)groups + off))
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
    IDirect3DRMFrame *BuildFrame(IDirect3DRM *d3drm, BOOL perspective);
};
IDirect3DRMFrame *SqgModel::BuildFrame(IDirect3DRM *d3drm, BOOL perspective)
{
    IDirect3DRMFrame *frame;
    int j, g, off, i, v, f, n, skip, pos;
    d3drm->CreateFrame(NULL, &frame);
    g = 0;
    if (groupCount > 0) {
        off = 0;
        do {
            IDirect3DRMMeshBuilder *builder;
            d3drm->CreateMeshBuilder(&builder);
            builder->SetName("PZMorph MeshBuilder");
            builder->SetPerspective(perspective);
            builder->ReserveSpace(G->vertexCount, G->vertexCount, G->faceCount);
            if (G->vertexCount != 0) {
                j = G->indexStart;
                skip = 0;
                for (i = 0; i < G->indexCount; i++, j++) {
                    if (skip == 0)
                        skip = indices[j];
                    else {
                        skip--;
                        indices[j] -= G->firstVertex;
                    }
                }
                for (v = 0; v < G->vertexCount; v++) {
                    builder->AddVertex(vertices[G->firstVertex + v].position.x, vertices[G->firstVertex + v].position.y, vertices[G->firstVertex + v].position.z);
                    builder->AddNormal(vertices[G->firstVertex + v].normal.x, vertices[G->firstVertex + v].normal.y, vertices[G->firstVertex + v].normal.z);
                    builder->SetTextureCoordinates(v, vertices[G->firstVertex + v].tu, vertices[G->firstVertex + v].tv);
                }
                pos = 0;
                for (f = 0; f < G->faceCount; f++) {
                    IDirect3DRMFace *face;
                    n = indices[G->indexStart + pos];
                    pos++;
                    builder->CreateFace(&face);
                    for (; n > 0; n--) {
                        int idx = indices[G->indexStart + pos];
                        pos++;
                        face->AddVertexAndNormalIndexed(idx, idx);
                    }
                    if (G->textureIndex != -1)
                        face->SetColor(0xffffffff);
                    else
                        face->SetColor(G->color);
                    face->Release();
                }
                for (i = 0, j = G->indexStart; i < G->indexCount; i++, j++) {
                    if (skip == 0)
                        skip = indices[j];
                    else {
                        skip--;
                        indices[j] += G->firstVertex;
                    }
                }
                TextureLookupFn lookup = GetTextureLookup();
                if (G->textureIndex != -1 && lookup)
                    builder->SetTexture(lookup(textureNames[G->textureIndex]));
                frame->AddVisual(builder);
                builder->Release();
            }
            g++;
            off += 0x24;
        } while (g < groupCount);
    }
    return frame;
}
