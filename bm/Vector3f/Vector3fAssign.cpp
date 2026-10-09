// Vector3f::operator= (0x4183c0): the vector copy, ~46 callers. Isolated in the original.
#include "Vector3f.h"
Vector3f &Vector3f::operator=(const Vector3f &o)
{
    x = o.x;
    y = o.y;
    z = o.z;
    return *this;
}
