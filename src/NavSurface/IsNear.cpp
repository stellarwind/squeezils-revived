// SQZ.EXE 0x40c700: is a world-space point within the surface's cached radius?
#include "NavSurface/Nearness.h"

int NearSurfaceInstance::IsNear(const Vector3f &point)
{
    Vector3f delta = position - point;
    float squared = delta.x * delta.x + delta.z * delta.z + delta.y * delta.y;
    // Original x87 C0 test rejects unordered values as well as points outside.
    return squared <= GetSurface()->sizeSquared;
}
