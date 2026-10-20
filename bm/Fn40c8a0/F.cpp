// 0x40c8a0: PointToLocal - (point - position) turned by the inverse of the matrix's turn; identity returns the point unchanged.
#include "Vector3f/Vector3f.h"
#include "Matrix4/Matrix4.h"
Vector3f *TransformPoint(Vector3f *out, const Vector3f *p, const Matrix4 *m);     // 0x47e1e0 (cdecl)
struct Frame4 { Matrix4 m; Matrix4 &Transpose(); };                                // 0x47dbf0 (on a Matrix4 copy)
Vector3f *PointToLocal(Vector3f *out, const Vector3f *point, const Matrix4 *m, int identity, const Matrix4 *inverse)
{
    if (identity) {
        out->x = point->x;
        out->y = point->y;
        out->z = point->z;
        return out;
    }
    if (!inverse) {
        Matrix4 turn = *m;
        Vector3f zero(0.0f, 0.0f, 0.0f);
        turn.SetTranslation(&zero.x);
        ((Frame4 *)&turn)->Transpose();
        Vector3f row = *(const Vector3f *)&m->m[12];
        Vector3f moved = *point + row * -1.0f;
        Vector3f local = *TransformPoint(&zero, &moved, &turn);
        out->x = local.x;
        out->y = local.y;
        out->z = local.z;
        return out;
    }
    Vector3f zero2(0.0f, 0.0f, 0.0f);
    Vector3f moved = *point + (*(const Vector3f *)&m->m[12]) * -1.0f;
    Vector3f local = *TransformPoint(&zero2, &moved, inverse);
    out->x = local.x;
    out->y = local.y;
    out->z = local.z;
    return out;
}
