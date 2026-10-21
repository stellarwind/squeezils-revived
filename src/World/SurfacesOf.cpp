#include "Runtime/DynamicCast.h"
// World::SurfacesOf, SQZ.EXE 0x4365a0.
#include "World/World.h"

struct WorldMatrix {
    float m[16];
    WorldMatrix() {}
    ~WorldMatrix(); // 0x402310
};
struct WorldContact {
    int face, second;
    WorldMatrix matrix;
    WorldContact(int id, int sub) { face = id; second = sub; }
    ~WorldContact(); // 0x4031e0
};
struct WorldSurfaces {
    NavMesh *FaceMesh(WorldContact *contact); // 0x412bc0
};
struct WorldPlace {
    WorldSurfaces *GetSurfaces(); // 0x439b60
};
void *GetSimulation(); // 0x420690
extern const char kWorldSimulationType[]; // 0x4ba1d0
extern const char kWorldPlaceType[];      // 0x4bae38

NavMesh *World::SurfacesOf(int face)
{
    WorldContact contact(face, 0);
    return ((WorldPlace *)SqzRTDynamicCast(GetSimulation(), 0,
        kWorldSimulationType, kWorldPlaceType, 0))->GetSurfaces()->FaceMesh(&contact);
}
