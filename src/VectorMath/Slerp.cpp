// 0x47da80: interpolate two unit vectors along their great circle.
#define V3_COPYCTOR
#include "Vector3f/Vector3f.h"
#include <math.h>
#pragma intrinsic(acos, sin, fabs)
Vector3f Slerp(float t, const Vector3f &a, const Vector3f &b)
{
    float angle = b.x * a.x + b.z * a.z + b.y * a.y;
    if (angle > 1.0f)
        angle = 1.0f;
    angle = (float)acos(angle);
    if (fabs(angle) < (double)0.001f)
        return a;
    double sine = sin(angle);
    Vector3f result = a * (float)(sin((1.0f - t) * angle) / sine)
        + b * (float)(sin(t * angle) / sine);
    return result;
}
