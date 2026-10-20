// SQZ.EXE 0x47d940: project a point onto the infinite line through a and b.
#define V3_COPYCTOR
#include "Vector3f/Vector3f.h"

Vector3f ClosestOnLine(const Vector3f &point, const Vector3f &a, const Vector3f &b)
{
    Vector3f direction = b - a;
    direction.Normalize();
    Vector3f result = a + direction * -(
        (a.x * direction.x + a.y * direction.y + a.z * direction.z +
         -(point.x * direction.x + point.y * direction.y + point.z * direction.z)) /
        (direction.x * direction.x + direction.z * direction.z + direction.y * direction.y));
    return result;
}
