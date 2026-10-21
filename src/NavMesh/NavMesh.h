// Shared navigation storage and loading interface.
#pragma once
#include <stdio.h>
#include "Vector3f/Vector3f.h"
#include "Vector3fEmpty/Vector3fEmpty.h"
struct MV3 { float x, y, z; MV3() {} MV3(float a, float b, float c) : x(a), y(b), z(c) {} ~MV3(); MV3 &operator=(const MV3 &o) { x = o.x; y = o.y; z = o.z; return *this; } };   // dtor 0x401660
typedef Vector3fEmpty MVert;
#include "NavMesh/BoxNode.h"
typedef BoxNode TreeNode;
struct MFace {                                                                                           // 0x28 bytes
    int a, b, c;            // 0x00
    MV3 normal;             // 0x0c
    int i0, i1, i2;         // 0x18
    int pad;                // 0x24
    MFace();                // 0x47ad20
    ~MFace();               // 0x4053e0
};

struct BoxNode;
// SIZE 0x2030: no virtual base; points/faces/tree at +4/+c/+10.
struct NavMesh {
    unsigned pointCount; // 0x00
    MVert *points; // 0x04
    unsigned faceCount; // 0x08
    MFace *faces; // 0x0c
    void *tree; // 0x10
    char hitStorage[0x201c - 0x14]; // 0x14
    int p201c; // 0x201c
    float radius; // 0x2020, squared radius
    int p2024, p2028, p202c; // 0x2024, 0x2028, 0x202c
    NavMesh(); // 0x47aec0, Misc15/Mesh.cpp
    ~NavMesh(); // 0x47af00, NavMeshDtor.cpp
    float Radius(); // 0x47af20
    int AllocPoints(int count); // 0x47af30
    int AllocFaces(int count); // 0x47aff0
    void FreeVerts(); // 0x47b0b0
    void FreeFaces(); // 0x47b0f0
    void FreeArrays(); // 0x47b130
    void FreeTree(); // 0x47b150
    int DetectFormat(const char *path); // 0x47c7b0
    int Load(const char *path); // 0x47c8e0
    int ReadAscii(FILE *file); // 0x47ca10
    int ReadBinary(FILE *file); // 0x47cfe0
    int ReadAsciiBox(FILE *file, BoxNode **out); // 0x47cc60
    int ReadBinaryBox(FILE *file, BoxNode **out); // 0x47d2b0
    BoxNode *FindLeaf(const Vector3f *point, BoxNode *box); // 0x47d560
    int InsideFace(MFace *face, const Vector3f &point); // 0x47c2a0
    int Raycast(const Vector3f &origin, int unused1, int unused2, const Vector3f &up,
        const Vector3f &dir, float length, Vector3f *point, Vector3f *right,
        Vector3f *normal, Vector3f *forward, int *face, const int *typeAllowed); // 0x47b180
};
typedef char NavMeshSizeCheck[(sizeof(NavMesh) == 0x2030) ? 1 : -1];
typedef char NavPointSizeCheck[(sizeof(MVert) == 0x0c) ? 1 : -1];
typedef char NavFaceSizeCheck[(sizeof(MFace) == 0x28) ? 1 : -1];
