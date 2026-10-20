// 0x47dea0: Matrix4 product (this * o), returned by value.
#include "compat41.h"
struct M4 { float m[16]; M4() {} ~M4() {} M4 operator*(const M4 &o) const; };
M4 M4::operator*(const M4 &o) const
{
    M4 r;
    r.m[0] = o.m[0] * m[0] + o.m[4] * m[1] + o.m[8] * m[2] + o.m[12] * m[3];
    r.m[1] = o.m[1] * m[0] + o.m[5] * m[1] + o.m[9] * m[2] + o.m[13] * m[3];
    r.m[2] = o.m[2] * m[0] + o.m[6] * m[1] + o.m[10] * m[2] + o.m[14] * m[3];
    r.m[3] = o.m[3] * m[0] + o.m[7] * m[1] + o.m[11] * m[2] + o.m[15] * m[3];
    r.m[4] = o.m[0] * m[4] + o.m[4] * m[5] + o.m[8] * m[6] + o.m[12] * m[7];
    r.m[5] = o.m[1] * m[4] + o.m[5] * m[5] + o.m[9] * m[6] + o.m[13] * m[7];
    r.m[6] = o.m[2] * m[4] + o.m[6] * m[5] + o.m[10] * m[6] + o.m[14] * m[7];
    r.m[7] = o.m[3] * m[4] + o.m[7] * m[5] + o.m[11] * m[6] + o.m[15] * m[7];
    r.m[8] = o.m[0] * m[8] + o.m[4] * m[9] + o.m[8] * m[10] + o.m[12] * m[11];
    r.m[9] = o.m[1] * m[8] + o.m[5] * m[9] + o.m[9] * m[10] + o.m[13] * m[11];
    r.m[10] = o.m[2] * m[8] + o.m[6] * m[9] + o.m[10] * m[10] + o.m[14] * m[11];
    r.m[11] = o.m[3] * m[8] + o.m[7] * m[9] + o.m[11] * m[10] + o.m[15] * m[11];
    r.m[12] = o.m[0] * m[12] + o.m[4] * m[13] + o.m[8] * m[14] + o.m[12] * m[15];
    r.m[13] = o.m[1] * m[12] + o.m[5] * m[13] + o.m[9] * m[14] + o.m[13] * m[15];
    r.m[14] = o.m[2] * m[12] + o.m[6] * m[13] + o.m[10] * m[14] + o.m[14] * m[15];
    r.m[15] = o.m[3] * m[12] + o.m[7] * m[13] + o.m[11] * m[14] + o.m[15] * m[15];
    return r;
}
