// SqgModel: a character / prop model loaded from an .SQG file ("SQG Format 1.0"), with morph targets.
// The game builds a D3DRM Mesh from it (one group per sub-mesh) and animates it by morphing: every frame the base
// vertex positions are copied, each active morph target adds its weighted deltas, and Mesh::SetVertices uploads the result.
// Reconstructed from SQZ.EXE 0x472560-0x473710. "SqgModel" / "PZMorphMesh" are our names (the game's debug strings call
// the mesh "PZMorph Mesh"). Field offsets are the original ones.
#pragma once
#define DIRECT3D_VERSION 0x0500
#include <windows.h>
#include <d3drm.h>

struct SqgGroup {                 // 0x24 bytes, stored in the file
    int color;                    // 0x00 used as the group colour when textureIndex is -1
    int f04, f08;                 // 0x04, 0x08 not used by the code reconstructed so far
    int textureIndex;             // 0x0c index into textureNames, -1 for none
    int firstVertex;              // 0x10 first vertex of this group in the model's vertex array
    int vertexCount;              // 0x14 number of vertices in the group (0: group is skipped)
    int faceCount;                // 0x18
    int indexStart;               // 0x1c where the group's face data starts in the index array
    int indexCount;               // 0x20 number of entries (face lists: count followed by that many vertex indices)
};

struct SqgDelta {                 // 0x10 bytes
    int   vertex;                 // vertex the delta applies to
    float dx, dy, dz;
};

struct SqgDeltaList {             // 8 bytes: one morph target
    int       count;
    SqgDelta *deltas;
};

// The Mesh built from a model plus the per-instance vertex copy that morphing writes into (0xc bytes).
struct PZMorphMesh {
    int           *groupIds;      // 0x00 D3DRM group id of every model group
    D3DRMVERTEX   *vertices;      // 0x04 working copy of the model's vertices
    IDirect3DRMMesh *mesh;        // 0x08
};

class SqgModel {
public:
    BOOL         Load(const char *path, int mirror);                              // 0x472560 mirror: flip X and the winding
    PZMorphMesh *CreateMesh(IDirect3DRM *d3drm);                                  // 0x472f60 (thiscall, 1 stack arg)
    void         Morph(PZMorphMesh *inst, int count, const float *weights);       // 0x4734e0 (thiscall, 3 stack args)
    // 0x4731f0 (thiscall, 2 stack args): a new frame holding one MeshBuilder visual per group. This is the path the game
    // uses for its models (UseMeshForGeomResources is off), and the source of the MeshBuilder call volume.
    IDirect3DRMFrame *BuildFrame(IDirect3DRM *d3drm, BOOL perspective);

    int            textureCount;     // 0x00
    char         **textureNames;     // 0x04
    int            vertexCount;      // 0x08
    D3DRMVERTEX   *vertices;         // 0x0c
    int            faceCount;        // 0x10
    int            indexCount;       // 0x14
    int           *indices;          // 0x18 face lists: count, then that many vertex indices
    int            groupCount;       // 0x1c
    SqgGroup      *groups;           // 0x20
    int            deltaListCount;   // 0x24
    SqgDeltaList  *deltaLists;       // 0x28
};
