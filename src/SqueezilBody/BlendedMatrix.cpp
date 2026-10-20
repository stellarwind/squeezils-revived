// SQZ.EXE 0x4574b0: interpolate orientation rows, keeping the destination position.
#include "SqueezilBody/BodyUpdate.h"

struct BodyBlendVector {
    float x, y, z;
    BodyBlendVector() {}
    BodyBlendVector(const float *v) : x(v[0]), y(v[1]), z(v[2]) {}
    ~BodyBlendVector(); // 0x401660
    BodyBlendVector &operator=(const BodyBlendVector &other) {
        x=other.x; y=other.y; z=other.z; return *this;
    }
    BodyBlendVector operator*(float scale) const; // 0x401b60
    BodyBlendVector operator+(const BodyBlendVector &other) const; // 0x403070
};

BodyUpdateMatrix UpdatingSqueezilBody::BlendedMatrix()
{
    BodyUpdateMatrix result;
    BodyBlendVector position(blendTo.m + 12), right, up, forward;
    float t = blendTime / gBlendSeconds;
    float s = 1.0f - t;
    right = *(BodyBlendVector *)(blendFrom.m) * s + *(BodyBlendVector *)(blendTo.m) * t;
    up = *(BodyBlendVector *)(blendFrom.m + 4) * s + *(BodyBlendVector *)(blendTo.m + 4) * t;
    forward = *(BodyBlendVector *)(blendFrom.m + 8) * s + *(BodyBlendVector *)(blendTo.m + 8) * t;
    result.SetRows(right, up, forward, position);
    result.Orthonormalize();
    return result;
}
