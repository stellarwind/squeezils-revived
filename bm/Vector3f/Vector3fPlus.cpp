// Vector3f::operator+ (0x403070). Isolated in the original; x87 scheduling order not matched yet.
#include "Vector3f.h"
Vector3f Vector3f::operator+(const Vector3f &o) const { return Vector3f(o.x + x, y + o.y, z + o.z); }
