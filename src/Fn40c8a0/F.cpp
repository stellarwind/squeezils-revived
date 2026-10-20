// 0x40c8a0: PointToLocal - (point - position) turned by the inverse of the matrix's turn; identity returns the point unchanged.
#include "Vector3f/Vector3f.h"
#include "Matrix4/Matrix4.h"
Vector3f *TransformPoint(Vector3f *out, const Vector3f *p, const Matrix4 *m);     // 0x47e1e0 (cdecl)
struct Frame4 { Matrix4 m; Matrix4 &Transpose(); };                                // 0x47dbf0 (on a Matrix4 copy)
struct Mx { float a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
    Mx(const Mx &o) : a0(o.a0), a1(o.a1), a2(o.a2), a3(o.a3), a4(o.a4), a5(o.a5), a6(o.a6), a7(o.a7), a8(o.a8), a9(o.a9), a10(o.a10), a11(o.a11), a12(o.a12), a13(o.a13), a14(o.a14), a15(o.a15) {}
    ~Mx() {} };
Vector3f __stdcall PointToLocal(const Vector3f &point, const Matrix4 *m, int identity, const Matrix4 *inverse)
{
    if (identity)
        return point;
    if (!inverse) {
        Matrix4 turn = *m;
        Vector3f zero(0.0f, 0.0f, 0.0f);
        turn.SetTranslation(&zero.x);
        ((Frame4 *)&turn)->Transpose();
        Vector3f moved = point + *(const Vector3f *)&m->m[12] * -1.0f;
        Vector3f local = *TransformPoint(&zero, &moved, &turn);
        return local;
    }
    Vector3f zero2(0.0f, 0.0f, 0.0f);
    Vector3f moved = point + *(const Vector3f *)&m->m[12] * -1.0f;
    Vector3f local = *TransformPoint(&zero2, &moved, inverse);
    return local;
}
