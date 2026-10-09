// Vector3f::Cross (0x41a350). Isolated in the original; x87 scheduling order not matched yet.
#include "Vector3f.h"
Vector3f Vector3f::Cross(const Vector3f &o) const { return Vector3f(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x); }
