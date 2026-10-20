// Vector3f 3-arg constructor as a real function (0x418570). The operators inline the same ctor, so this unit
// defines it out-of-line (V3_CTOR_OUTOFLINE) to match the original's separate real copy.
#define V3_CTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"

Vector3f::Vector3f(float ax, float ay, float az)
{
    x = ax;
    y = ay;
    z = az;
}
