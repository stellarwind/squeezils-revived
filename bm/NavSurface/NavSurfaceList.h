// NavSurfaceList: the place's surface list (SQZ.EXE 0x40bf90-0x412c40), the two trivial Array-getter leaves ported.
// Layout from Repo/src/NavSurface.h. The object is PlaceSimulation + 0x3c.
#pragma once
#include "Container/ContainerArray.h"

struct LoadedSurface;
struct NavMesh;

struct InstancedSurface {
    char pad98[0x98];
    void *GetProperties();    // 0x40d520 (non-inline): returns this + 0x98
};

struct Contact {
    int   face;          // 0x00 the surface face, -1 for none
    int   second;        // 0x04
    float matrix[16];    // 0x08
};

struct NavSurfaceList {
    char                         pad1[0x124];   // 0x00 .. 0x124
    Array<LoadedSurface *>       loaded;        // 0x124
    Array<InstancedSurface *>    instances;     // 0x134

    LoadedSurface    *Loaded(int number);           // 0x412c10
    InstancedSurface *FaceOf(const Contact *contact);   // 0x412bf0
    void *Properties(const Contact *c);             // 0x40f3a0
    NavMesh *FaceMesh(const Contact *c);             // 0x412bc0
    int FaceType(const Contact *c);                 // 0x40f380
};
