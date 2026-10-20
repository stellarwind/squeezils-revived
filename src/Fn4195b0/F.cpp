// SQZ.EXE 0x4195b0 (ret 4, result by hidden pointer): build the 4x4 look matrix of a placement from its forward/up vectors and position.
#include "Vector3f/Vector3f.h"

struct LMat {                                            // 64 bytes, empty out-of-line destructor 0x402310
    float v[16];
    LMat() {}
    LMat(const Vector3f &r, const Vector3f &t, const Vector3f &f, const Vector3f &p)
    {
        v[0] = r.x;
        v[1] = r.y;
        v[2] = r.z;
        v[3] = 0;
        v[4] = t.x;
        v[5] = t.y;
        v[6] = t.z;
        v[7] = 0;
        v[8] = f.x;
        v[9] = f.y;
        v[10] = f.z;
        v[11] = 0;
        v[12] = p.x;
        v[13] = p.y;
        v[14] = p.z;
        v[15] = 1.0f;
    }
    LMat(const LMat &o)
    {
        v[0] = o.v[0];
        v[1] = o.v[1];
        v[2] = o.v[2];
        v[3] = o.v[3];
        v[4] = o.v[4];
        v[5] = o.v[5];
        v[6] = o.v[6];
        v[7] = o.v[7];
        v[8] = o.v[8];
        v[9] = o.v[9];
        v[10] = o.v[10];
        v[11] = o.v[11];
        v[12] = o.v[12];
        v[13] = o.v[13];
        v[14] = o.v[14];
        v[15] = o.v[15];
    }
    ~LMat();
};

struct LookFrame {
    Vector3f position;              // 0x00
    Vector3f forward;               // 0x0c
    Vector3f up;                    // 0x18
    LMat Matrix();
};

LMat LookFrame::Matrix()
{
    Vector3f right = up.Cross(forward);
    right.Normalize();
    Vector3f top = forward.Cross(right);
    top.Normalize();
    LMat m(right, top, forward, position);
    return m;
}
