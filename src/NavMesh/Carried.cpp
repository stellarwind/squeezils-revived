// SQZ.EXE 0x40d370: displacement caused by moving the supporting surface.
#include "NavMesh/SurfaceTransform.h"

Vector3f TransformWorldPoint(const Vector3f &point, const Matrix16 &matrix); // 0x47e1e0

Vector3f InstancedSurface::Carried(const Vector3f &point)
{
    Vector3f local = ToLocal(point);
    Matrix16 rotation(previous);
    rotation.SetTranslation(Vector3f(0.0f, 0.0f, 0.0f));
    Vector3f was = TransformWorldPoint(local, rotation);
    was = was + previous.Position();
    return point - was;
}
