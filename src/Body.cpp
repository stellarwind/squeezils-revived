// Body: see Body.h. Reconstructed from SQZ.EXE 0x41fb90-0x4200fd.
#include <string.h>
#include "Body.h"
#include "Vector3f.h"

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

// @0040bae0
void Body::Destruct()
{
    vtbl = (void **)0x49d484;
}

// @0047e350
void OrthonormalizeMatrix(float *m)
{
    Vector3f up(m[4], m[5], m[6]);
    Vector3f facing(m[8], m[9], m[10]);
    Vector3f position(m[12], m[13], m[14]);
    Vector3f right = Cross(up, facing);
    Normalize(right);
    Normalize(up);
    facing = Cross(right, up);
    m[0] = right.x;  m[1] = right.y;  m[2] = right.z;   m[3] = 0.0f;
    m[4] = up.x;     m[5] = up.y;     m[6] = up.z;      m[7] = 0.0f;
    m[8] = facing.x; m[9] = facing.y; m[10] = facing.z; m[11] = 0.0f;
    m[12] = position.x; m[13] = position.y; m[14] = position.z; m[15] = 1.0f;
}
