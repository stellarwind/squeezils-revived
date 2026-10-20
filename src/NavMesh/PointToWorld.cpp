// SQZ.EXE 0x40ce30: transform a point using an optional precomputed rotation.
#include "NavMesh/SurfaceTransform.h"

Vector3f TransformWorldPoint(const Vector3f &point, const Matrix16 &matrix); // 0x47e1e0, cdecl

Vector3f InstancedSurface::PointToWorld(const Vector3f &point, const Matrix16 &matrix,
                                       int identity, const Matrix16 *turn)
{
    if (identity)
        return point;
    if (!turn) {
        Matrix16 rotation(matrix);
        rotation.SetTranslation(Vector3f(0.0f, 0.0f, 0.0f));
        Vector3f result = TransformWorldPoint(point, rotation);
        result = result + matrix.Position();
        return result;
    } else {
        Vector3f result = TransformWorldPoint(point, *turn);
        result = result + matrix.Position();
        return result;
    }
}
