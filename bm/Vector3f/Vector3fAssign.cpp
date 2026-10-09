// Vector3f::operator= as a real function (0x4183c0): the vector copy, isolated in the original.
// Inlined copies at other call sites come from the inline body in Vector3f.h.
#define V3_ASSIGN_OUTOFLINE
#include "Vector3f/Vector3f.h"

Vector3f &Vector3f::operator=(const Vector3f &o)
{
    x = o.x;
    y = o.y;
    z = o.z;
    return *this;
}
