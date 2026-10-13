// 16 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
void PadDecl16();
// Transform a point by a 4x4 matrix with perspective divide (SQZ.EXE 0x47e1e0, ret via hidden pointer, cdecl-like 3 args).
#include "compat41.h"

struct V3 {
    float x, y, z;
    V3() {}
    V3(float a, float b, float c) : x(a), y(b), z(c) {}
    V3(const V3 &o) : x(o.x), y(o.y), z(o.z) {}
    ~V3();                                   // 0x401660
    V3 operator/(float s) const;             // 0x420100
};
struct Mat4 { float m[16]; };

V3 TransformPoint(const V3 *p, const Mat4 *mat)
{
    const float *m = mat->m;
    V3 v;
    v.x = p->z * m[8] + p->y * m[4] + p->x * m[0] + m[12];
    v.y = p->x * m[1] + p->y * m[5] + p->z * m[9] + m[13];
    v.z = p->z * m[10] + p->y * m[6] + p->x * m[2] + m[14];
    float w = p->z * m[11] + p->y * m[7] + p->x * m[3] + m[15];
    return w == 1.0 ? v : v / w;
}
