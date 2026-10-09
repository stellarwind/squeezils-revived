// SqueezilBody trivial getters/setters (0x457ac0, 0x458fe0, 0x458ff0).
#include "SqueezilBody/SqueezilBody.h"

int SqueezilBody::GetMode()         { return mode; }
int SqueezilBody::IsOnGround()      { return onGround; }
void SqueezilBody::SetOnGround(int on) { onGround = on; }

void SqueezilBody::SetOrientation(float pitch, float yaw, float roll)
{
    orientation.x = pitch;
    orientation.y = yaw;
    orientation.z = roll;
}

void SqueezilBody::GetOrientation(float *pitch, float *yaw, float *roll)
{
    *pitch = orientation.x;
    *yaw = orientation.y;
    *roll = orientation.z;
}
