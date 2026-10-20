// SQZ.EXE 0x4128a0: transform a point using the surface's previous placement.
#include "NavMesh/SurfaceTransform.h"

Vector3f InstancedSurface::ToWorldBefore(const Vector3f &point)
{
    return PointToWorld(point, previous, previousIdentity, 0);
}
