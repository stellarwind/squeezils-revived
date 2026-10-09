// Body getters (0x40bb70-0x40bba0, 0x41fcb0).
#include "Body/Body.h"

float *Body::GetMatrix()      { return matrix; }
Float3 *Body::GetFacing()     { return &facing; }
float Body::GetFacingValue()  { return facingValue; }
float Body::GetExtraValue()   { return extraValue; }
Contact *Body::GetContact()   { return &contact; }
int Body::IsOnSurface()       { return 0; }

void Body::SetFacing(const Float3 *direction, float value)
{
    facing.x = direction->x;
    facing.y = direction->y;
    facing.z = direction->z;
    facingValue = value;
}

void Body::SetAngles(float x, float y, float z)
{
    angles.x = x;
    angles.y = y;
    angles.z = z;
}

void Body::GetAngles(float *x, float *y, float *z)
{
    *x = angles.x;
    *y = angles.y;
    *z = angles.z;
}

void Body::SetMatrix(const float *m)
{
    for (int i = 0; i < 16; i++)
        matrix[i] = m[i];
}

void Body::SetContact(const Contact *c)
{
    contact = *c;
}
