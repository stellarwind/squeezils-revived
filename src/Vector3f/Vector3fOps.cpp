// Vector3f::operator- (0x401ae0) and operator* (0x401b60): adjacent in the original, one translation unit.
#include "Vector3f.h"
Vector3f Vector3f::operator-(const Vector3f &o) const { return Vector3f(x - o.x, y - o.y, z - o.z); }
Vector3f Vector3f::operator*(float s) const { return Vector3f(x * s, y * s, z * s); }
