// Vector3f::Length (0x47d5f0), LengthSq (0x47d610), Normalize (0x47d630): contiguous in the original, one unit.
#include <math.h>
#include "Vector3f.h"
float Vector3f::Length() const
{
    float sum = z * z;
    sum += y * y;
    sum += x * x;
    return (float)sqrt(sum);
}
float Vector3f::LengthSq() const
{
    float sum = x * x;
    sum += y * y;
    sum += z * z;
    return sum;
}
float Vector3f::Normalize()
{
    float length = Length();
    if (length != 0.0) {
        float scale = 1.0f / length;
        x *= scale;
        y *= scale;
        z *= scale;
    }
    return length;
}
