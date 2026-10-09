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
    matrix[0] = m[0];
    matrix[1] = m[1];
    matrix[2] = m[2];
    matrix[3] = m[3];
    matrix[4] = m[4];
    matrix[5] = m[5];
    matrix[6] = m[6];
    matrix[7] = m[7];
    matrix[8] = m[8];
    matrix[9] = m[9];
    matrix[10] = m[10];
    matrix[11] = m[11];
    matrix[12] = m[12];
    matrix[13] = m[13];
    matrix[14] = m[14];
    matrix[15] = m[15];
}

void Body::SetContact(const Contact *c)
{
    contact.face = c->face;
    contact.second = c->second;
    contact.matrix[0] = c->matrix[0];
    contact.matrix[1] = c->matrix[1];
    contact.matrix[2] = c->matrix[2];
    contact.matrix[3] = c->matrix[3];
    contact.matrix[4] = c->matrix[4];
    contact.matrix[5] = c->matrix[5];
    contact.matrix[6] = c->matrix[6];
    contact.matrix[7] = c->matrix[7];
    contact.matrix[8] = c->matrix[8];
    contact.matrix[9] = c->matrix[9];
    contact.matrix[10] = c->matrix[10];
    contact.matrix[11] = c->matrix[11];
    contact.matrix[12] = c->matrix[12];
    contact.matrix[13] = c->matrix[13];
    contact.matrix[14] = c->matrix[14];
    contact.matrix[15] = c->matrix[15];
}
