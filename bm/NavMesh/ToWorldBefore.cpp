// SQZ.EXE 0x4128a0: transform a point using the surface's previous placement.
#include "compat41.h"

struct Vector3f {
    float x, y, z;
    Vector3f() {}
    ~Vector3f();                             // 0x401660
};
struct Matrix16 { float v[16]; };

struct InstancedSurface {
    char pad00[0x48];
    Matrix16 previous;                      // +0x48
    int previousIdentity;                   // +0x88
    // 0x40ce30: member helper with four arguments and a hidden result pointer.
    Vector3f PointToWorld(const Vector3f &point, const Matrix16 &matrix,
                          int identity, const Matrix16 *turn);
    Vector3f ToWorldBefore(const Vector3f &point);
};

Vector3f InstancedSurface::ToWorldBefore(const Vector3f &point)
{
    return PointToWorld(point, previous, previousIdentity, 0);
}
