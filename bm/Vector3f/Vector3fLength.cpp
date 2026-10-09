// Vector3f::Length (0x47d5f0), LengthSq (0x47d610), Normalize (0x47d630): contiguous in the original, one unit.
#include <math.h>
#include "Vector3f.h"
float Vector3f::Length() const { return (float)sqrt(x * x + y * y + z * z); }
float Vector3f::LengthSq() const { return x * x + y * y + z * z; }
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
