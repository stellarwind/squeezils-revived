// Body: see Body.h. Reconstructed from SQZ.EXE 0x41fb90-0x4200fd.
#include <string.h>
#include "Body.h"

// @0041fb90
Body *Body::Init(const float *m)
{
    contact.face = -1;
    contact.second = 0;
    vtbl = (void **)0x49d484;
    memcpy(matrix, m, sizeof matrix);
    facing.x = matrix[8];
    facing.y = matrix[9];
    facing.z = matrix[10];
    SetAngles(0.0f, 0.0f, 0.0f);
    facingValue = 0.0f;
    extraValue = 0.0f;
    return this;
}

// @0041fcc0
void Body::SetMatrix(const float *m)
{
    memcpy(matrix, m, sizeof matrix);
}

// @0041fd30
void Body::SetFacing(const Float3 *direction, float value)
{
    facing = *direction;
    facingValue = value;
}

// @0041fd60
void Body::SetAngles(float x, float y, float z)
{
    angles.x = x;
    angles.y = y;
    angles.z = z;
}

// @0041fd80
void Body::GetAngles(float *x, float *y, float *z)
{
    *x = angles.x;
    *y = angles.y;
    *z = angles.z;
}

// @00420060
void Body::SetContact(const Contact *c)
{
    contact = *c;
}
