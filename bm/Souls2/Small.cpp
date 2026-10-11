// PhysicalSettings::SetGravity (0x446dd0) and the matrix maker (0x412830).
#include <math.h>

struct PhysSet {
    float gx, gy, gz;          // gravity
    float dx, dy, dz;          // down
    float strength;
    float SetGravity(float g);
};

float PhysSet::SetGravity(float g)
{
    gx = 0.0f;
    gy = g;
    gz = 0.0f;
    float sign = 1.0f;
    if (*(unsigned *)&g > 0x80000000)
        sign = -1.0f;
    strength = (float)fabs(g);
    dx = 0.0f;
    dy = sign;
    dz = 0.0f;
    return g;
}

struct Mat16 {
    float m[16];
    Mat16 *Make(const float *right, const float *up, const float *forward, const float *pos);
};

Mat16 *Mat16::Make(const float *right, const float *up, const float *forward, const float *pos)
{
    m[0] = right[0];
    m[1] = right[1];
    m[2] = right[2];
    m[3] = 0.0f;
    m[4] = up[0];
    m[5] = up[1];
    m[6] = up[2];
    m[7] = 0.0f;
    m[8] = forward[0];
    m[9] = forward[1];
    m[10] = forward[2];
    m[11] = 0.0f;
    m[12] = pos[0];
    m[13] = pos[1];
    m[14] = pos[2];
    m[15] = 1.0f;
    return this;
}
