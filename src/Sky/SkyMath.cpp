// SkyRenderer::RotationMatrix (0x48d7a0): rotation about one axis as a 3x3, row major (cdecl, no this).
#include <math.h>

void RotationMatrix(float *out, int axis, float angle)
{
    float s, c;
    c = (float)cos(angle);
    s = (float)sin(angle);
    for (int i = 0; i < 3; i++) {
        out[i * 3] = 0.0f;
        out[i * 3 + 1] = 0.0f;
        out[i * 3 + 2] = 0.0f;
    }
    out[0] = 1.0f; out[8] = 1.0f; out[4] = 1.0f;
    switch (axis) {
    case 0:
        out[7] = -s; out[4] = c; out[5] = s; out[8] = c;
        break;
    case 1:
        out[2] = -s; out[0] = c; out[6] = s; out[8] = c;
        break;
    case 2:
        out[3] = -s; out[0] = c; out[1] = s; out[4] = c;
        break;
    }
}

void MatrixMultiply(float *out, const float *a, const float *b)
{
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++)
            out[i * 3 + j] = a[j] * b[i * 3] + a[3 + j] * b[i * 3 + 1] + b[i * 3 + 2] * a[6 + j];
    }
}

void MulVector(float *out, const float *m, const float *v)
{
    out[0] = m[1] * v[1] + m[2] * v[2] + v[0] * m[0];
    out[1] = m[5] * v[2] + m[3] * v[0] + m[4] * v[1];
    out[2] = m[6] * v[0] + m[7] * v[1] + m[8] * v[2];
}
