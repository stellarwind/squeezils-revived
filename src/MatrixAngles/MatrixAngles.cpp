// 0x47e500: recover pitch, yaw and roll from the game's row-major transform.
#include <math.h>
#pragma intrinsic(sqrt, atan2, sin, cos, fabs)
struct AngleMatrix {
    float m[16];
    void GetAngles(float *pitch, float *yaw, float *roll);
};
void AngleMatrix::GetAngles(float *pitch, float *yaw, float *roll)
{
    float lift = -m[9];
    float rollY, rollX, yawY, yawX;
    if ((float)sqrt(1.0f - lift * lift) != 0.0f) {
        rollY = m[1];
        rollX = m[5];
        yawY = m[8];
        yawX = m[10];
    } else {
        rollY = m[2];
        rollX = m[6];
        yawY = 0;
        yawX = 1;
    }
    *roll = (float)atan2(rollY, rollX);
    *yaw = (float)atan2(yawY, yawX);
    yawY = (float)sin(*yaw);
    yawX = (float)cos(*yaw);
    if (fabs(yawX) < fabs(yawY))
        rollY = m[8] / yawY;
    else
        rollY = m[10] / yawX;
    *pitch = (float)atan2(lift, rollY);
}
