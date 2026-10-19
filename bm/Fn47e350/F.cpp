// SQZ.EXE 0x47e350: re-orthogonalise a 4x4 transform: row0 = row1 x row2 (unit), row1 normalised, row2 = row0 x row1; translation row kept.
#include "Vector3f/Vector3f.h"

struct Frame {
    Vector3f r0; float w0;          // 0x00
    Vector3f r1; float w1;          // 0x10
    Vector3f r2; float w2;          // 0x20
    Vector3f r3; float w3;          // 0x30
    Frame *Orthonormalize();
};

Frame *Frame::Orthonormalize()
{
    Vector3f x, a, b, c;
    a = r1;
    b = r2;
    c = r3;
    x = a.Cross(b);
    x.Normalize();
    a.Normalize();
    b = x.Cross(a);
    r0.x = x.x;
    r0.z = x.z;
    r0.y = x.y;
    w0 = 0;
    r1 = a;
    w1 = 0;
    r2 = b;
    w2 = 0;
    r3 = c;
    w3 = 1.0f;
    return this;
}
