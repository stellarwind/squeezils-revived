// Body: see Body.h. Reconstructed from SQZ.EXE 0x41fb90-0x4200fd.
#include <string.h>
#include "Body.h"
#include "Vector3f.h"
#include "Original.h"

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

// @0041fda0 Puts the body on the middle of a face. The face's frame: right = unit(normal x reference), up = normal, forward =
// right x normal, where the reference is the world's y axis, or the x axis when the face is nearly flat (|normal.y| > 0.9); its
// origin is the mean of the face's three corners. That frame (relative to the face) becomes the contact's matrix and, taken into
// the world, the body's own matrix.
void Body::PlaceOnFace(const Contact *at)
{
    char *surface = (char *)WorldSurface(GetWorld(), at->face);
    char *faces = *(char **)(surface + 0xc);
    Vector3f *corners = *(Vector3f **)(surface + 4);
    const int *face = (const int *)(faces + at->second * 0x28);

    Vector3f sum = corners[face[0]] + corners[face[1]] + corners[face[2]];
    Vector3f centre(sum.x / 3.0f, sum.y / 3.0f, sum.z / 3.0f);
    Vector3f normal = *(Vector3f *)(faces + at->second * 0x28 + 0xc);
    Vector3f reference = (normal.y > 0.9f || normal.y < -0.9f) ? Vector3f(1.0f, 0.0f, 0.0f) : Vector3f(0.0f, 1.0f, 0.0f);
    Vector3f right = Cross(normal, reference);
    Normalize(right);
    Vector3f forward = Cross(right, normal);

    float local[16] = { right.x, right.y, right.z, 0.0f, normal.x, normal.y, normal.z, 0.0f,
                        forward.x, forward.y, forward.z, 0.0f, centre.x, centre.y, centre.z, 1.0f };
    float world[16];
    memcpy(matrix, WorldFaceToWorld(GetWorld(), world, local, at->face), sizeof matrix);
    contact.face = at->face;
    contact.second = at->second;
    memcpy(contact.matrix, local, sizeof local);
}
