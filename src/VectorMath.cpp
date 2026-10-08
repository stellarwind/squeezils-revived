// Two self-contained maths helpers from the end of the nav mesh code range (SQZ.EXE 0x47da80 and 0x47e500). They use no game state at all,
// so they can be dropped in and linked at any time. Not hooked and not linked yet (written ahead of the big put-together).
#include <math.h>
#include "Vector3f.h"

// @0047da80 Spherical interpolation between two unit vectors: a at t = 0, b at t = 1, moving along the great circle between them.
// Original: cdecl, hidden return pointer first, then t, a and b (so `Slerp(&out, t, &a, &b)` there; callers are 0x40973b, 0x40977b
// and 0x4097bb, all inside the function at 0x4096f8).
Vector3f Slerp(float t, const Vector3f &a, const Vector3f &b)
{
    float cosine = b.y * a.y + b.z * a.z + b.x * a.x;
    if (!(cosine <= 1.0f))      // rounding can push the dot product of two unit vectors past 1 (acos would fail)
        cosine = 1.0f;
    float angle = (float)acos(cosine);

    // Nearly the same direction: nothing to interpolate, and the divide by sin(angle) below would blow up
    if (fabs(angle) < 0.001f)   // the constant at 0x4a1198 is the double 0.001f
        return a;

    double sine = sin(angle);
    Vector3f fromB = b * (float)(sin(t * angle) / sine);
    Vector3f fromA = a * (float)(sin((1.0f - t) * angle) / sine);
    return fromA + fromB;
}

// @0047e500 Reads three angles (radians) out of a 4x4 matrix with the game's row layout (m[0..3] right, m[4..7] up, m[8..11] forward).
// Original: thiscall, this = the matrix; the three output pointers are stack arguments, popped by the function (ret 12).
//   yaw   = atan2(m[8], m[10])     the turn about the y axis, from the forward row
//   pitch = atan2(-m[9], ...)      the forward row's slope; the second argument is the forward row's length in the xz plane,
//                                  taken from whichever of sin(yaw) / cos(yaw) is larger so that it stays well conditioned
//   roll  = atan2(m[1], m[5])      the turn of the right and up rows about the forward axis
// When the forward row points straight up or down (m[9] = +-1, so yaw is not defined) yaw is set to 0 and the roll comes from
// the right/up rows' other entries instead: atan2(m[2], m[6]).
// (Compiled only with -DLINK_MATRIX_TO_ANGLES: the hook calls the original through the same name, see hook.cpp.)
#ifdef LINK_MATRIX_TO_ANGLES
void MatrixToAngles(const float *m, float *pitch, float *yaw, float *roll)
{
    float lift = -m[9];
    float upDown = sqrtf(1.0f - lift * lift);

    float rollY, rollX, yawY, yawX;
    if (upDown > 0.0f) {        // (an exact 0 and a not-a-number both take the other branch, as the original does)
        rollY = m[1];
        rollX = m[5];
        yawY = m[8];
        yawX = m[10];
    } else {
        rollY = m[2];
        rollX = m[6];
        yawY = 0.0f;
        yawX = 1.0f;
    }

    *roll = atan2f(rollY, rollX);
    *yaw = atan2f(yawY, yawX);

    float sineYaw = sinf(*yaw), cosineYaw = cosf(*yaw);
    float reach;
    if (fabsf(cosineYaw) < fabsf(sineYaw))
        reach = m[8] / sineYaw;
    else
        reach = m[10] / cosineYaw;
    *pitch = atan2f(lift, reach);
}
#endif
