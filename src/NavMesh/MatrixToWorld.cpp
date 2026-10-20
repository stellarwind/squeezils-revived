// SQZ.EXE 0x40d070: compose local and surface rotations, then transform the position.
#include "NavMesh/SurfaceTransform.h"

SurfaceWorldMatrix InstancedSurface::MatrixToWorld(const Matrix16 &local,
    const Matrix16 &matrix, int identity)
{
    if (identity)
        return local;
    Matrix16 rotation(matrix);
    rotation.SetTranslation(Vector3f(0.0f, 0.0f, 0.0f));
    SurfaceWorldMatrix result(local);
    result.SetTranslation(Vector3f(0.0f, 0.0f, 0.0f));
    result = result * rotation;
    result.SetTranslation(PointToWorld(local.Position(), matrix, identity, &rotation));
    return result;
}
