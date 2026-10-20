// SQZ.EXE 0x40cb30: transform a world matrix into the surface's local coordinates.
#include "NavMesh/SurfaceTransform.h"

SurfaceWorldMatrix InstancedSurface::MatrixToLocal(const Matrix16 &world,
    const Matrix16 &matrix, int identity)
{
    if (identity)
        return world;
    Matrix16 inverse(matrix);
    inverse.SetTranslation(Vector3f(0.0f, 0.0f, 0.0f));
    inverse.Transpose();
    SurfaceWorldMatrix result(world);
    result.SetTranslation(Vector3f(0.0f, 0.0f, 0.0f));
    result = result * inverse;
    result.SetTranslation(PointToLocal(world.Position(), matrix, identity, &inverse));
    return result;
}
