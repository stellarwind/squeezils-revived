#include "Runtime/DynamicCast.h"
// 0x436640: apply a surface face's transform to a matrix.
struct FaceWorldMatrix {
    float m[16];
    FaceWorldMatrix() {}
    ~FaceWorldMatrix(); // 0x402310
};
struct FaceWorldContact {
    int face, second;
    FaceWorldMatrix matrix;
    FaceWorldContact(int id, int sub) { face = id; second = sub; }
    ~FaceWorldContact(); // 0x4031e0
};
struct WorldSurfaceFace {
    FaceWorldMatrix Apply(const FaceWorldMatrix &matrix); // 0x4105f0
};
struct FaceWorldSurfaces {
    WorldSurfaceFace *FaceOf(FaceWorldContact *contact); // 0x412bf0
};
struct FaceWorldPlace {
    FaceWorldSurfaces *GetSurfaces(); // 0x439b60
};
void *GetSimulation(); // 0x420690
extern const char kWorldSimulationType[]; // 0x4ba1d0
extern const char kWorldPlaceType[];      // 0x4bae38

struct FaceTransformWorld {
    FaceWorldMatrix FaceToWorld(const FaceWorldMatrix &matrix, int face);
};

FaceWorldMatrix FaceTransformWorld::FaceToWorld(const FaceWorldMatrix &matrix, int face)
{
    FaceWorldContact contact(face, 0);
    return ((FaceWorldPlace *)SqzRTDynamicCast(GetSimulation(), 0,
        kWorldSimulationType, kWorldPlaceType, 0))->GetSurfaces()->FaceOf(&contact)->Apply(matrix);
}
