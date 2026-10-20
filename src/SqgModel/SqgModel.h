// SqgModel: a model loaded from an .SQG or .WRL file.
#pragma once
#include "compat41.h"
struct PZMorphMesh;

struct SqgModel {
    int   textureCount;     // 0x00
    char **textureNames;    // 0x04
    int   vertexCount;      // 0x08
    void *vertices;         // 0x0c
    int   faceCount;        // 0x10
    int   indexCount;       // 0x14
    int  *indices;          // 0x18
    int   groupCount;       // 0x1c
    void *groups;           // 0x20
    int   deltaListCount;   // 0x24
    void *deltaLists;       // 0x28

    SqgModel *SqgModelConstruct();   // 0x4719d0 (thiscall)
    int LoadWrl(const char *path, int flag); // 0x472480
    int Load(const char *path, int mirror); // 0x472560
    void Morph(PZMorphMesh *instance, int count, const float *weights); // 0x4734e0
};
