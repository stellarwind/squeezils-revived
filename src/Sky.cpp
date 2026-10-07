// SkyRenderer: see Sky.h. Reconstructed from SQZ.EXE 0x48bb30-0x48e2ff.
//
// The original is x87 code: intermediate results stay in 80 bit registers and are only rounded where the C source stored
// them in a float. The long double / (float) casts below follow the original's rounding points so the texel positions
// come out the same.
#include "Sky.h"
#include "Original.h"
#include <math.h>
#include <string.h>


SkyRenderer g_sky;

// constants from the original's read-only data
static constexpr float  kDegToRad     = 0.017453f;        // 0x3c8ef998
static constexpr float  kHalfDegToRad = 0.0087265f;       // 0x3c0ef998 (also used negated)
static constexpr double kHalfDegToRadD = 0.0087265;       // 0x3f81df3300de4c51
static constexpr float  kInvPi        = 0.31830987f;      // 0x3ea2f983
static_assert(__builtin_bit_cast(unsigned, kDegToRad) == 0x3c8ef998u, "constant");
static_assert(__builtin_bit_cast(unsigned, kHalfDegToRad) == 0x3c0ef998u, "constant");
static_assert(__builtin_bit_cast(unsigned long long, kHalfDegToRadD) == 0x3f81df3300de4c51ull, "constant");
static_assert(__builtin_bit_cast(unsigned, kInvPi) == 0x3ea2f983u, "constant");

static inline int FloatBits(float f) { return __builtin_bit_cast(int, f); }

// ---- small 3x3 matrix helpers (row major) ----

// @0048d860
void SkyRenderer::MatrixMultiply(float *out, const float *a, const float *b)
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            out[i * 3 + j] = a[i * 3 + 2] * b[6 + j] + b[3 + j] * a[i * 3 + 1] + b[j] * a[i * 3 + 0];
}

// @0048d8b0 out = m * v. The three rows add their terms in different orders, as the original does.
static void MulVector(float *out, const float *m, const float *v)
{
    out[0] = v[0] * m[0] + m[2] * v[2] + m[1] * v[1];
    out[1] = m[4] * v[1] + m[3] * v[0] + m[5] * v[2];
    out[2] = m[8] * v[2] + m[7] * v[1] + m[6] * v[0];
}

// @0048d7a0
void SkyRenderer::RotationMatrix(float *out, int axis, float angle)
{
    float c = (float)cosl((long double)angle);
    float s = (float)sinl((long double)angle);
    memset(out, 0, 9 * sizeof(float));
    out[0] = 1.0f; out[8] = 1.0f; out[4] = 1.0f;
    if (axis == 0) {
        out[7] = -s; out[4] = c; out[5] = s; out[8] = c;
    } else if (axis == 1) {
        out[2] = -s; out[0] = c; out[6] = s; out[8] = c;
    } else if (axis == 2) {
        out[3] = -s; out[0] = c; out[1] = s; out[4] = c;
    }
}

// @0048d1a0 rotation matrix from three euler angles (radians). The partial products are kept as doubles.
static void EulerMatrix(const float *a, float *m)
{
    long double ca = cosl(a[0]), cb = cosl(a[1]), cc = cosl(a[2]);
    long double sa = sinl(a[0]), sb = sinl(a[1]), sc = sinl(a[2]);
    double p10 = (double)(cc * ca);
    double p08 = (double)(sc * ca);
    double p00 = (double)(sa * cc);
    double p18 = (double)(sc * sa);
    m[0] = (float)(cc * cb);
    m[1] = (float)((long double)p00 * sb - (long double)p08);
    m[2] = (float)((long double)p10 * sb + (long double)p18);
    m[3] = (float)(sc * cb);
    m[4] = (float)((long double)p18 * sb + (long double)p10);
    m[5] = (float)((long double)p08 * sb - (long double)p00);
    m[6] = (float)-sb;
    m[7] = (float)(sa * cb);
    m[8] = (float)(cb * ca);
}

// @0048d9b0 The rotation that turns the axis `dir` into +z, with a roll about z. Used for the face's own orientation.
// Returns false when `dir` is (nearly) zero.
static bool AxisRotation(const float *dir, float roll, float *out)
{
    float xx = dir[0] * dir[0];
    float zz = dir[2] * dir[2];
    float lenXZ = (float)sqrtl((long double)xx + zz);
    float a, b;                                    // -x / lenXZ and z / lenXZ
    if (lenXZ < 1e-5f && (unsigned)FloatBits(lenXZ) < 0xb727c5acu) {
        a = 0.0f;
        b = 1.0f;
    } else {
        a = (float)-((long double)dir[0] / lenXZ);
        b = (float)((long double)dir[2] / lenXZ);
    }
    float len = (float)sqrtl((long double)dir[1] * dir[1] + zz + xx);
    if (len < 1e-5f && (unsigned)FloatBits(len) < 0xb727c5acu)
        return false;
    float sy = (float)((long double)dir[1] / len);
    float cy = (float)((long double)lenXZ / len);

    float pitch[9] = { 1, 0, 0,   0, cy, sy,   0, -sy, cy };
    float yaw[9]   = { b, 0, a,   0, 1, 0,   -a, 0, b };
    float rollM[9], tmp[9];
    SkyRenderer::RotationMatrix(rollM, 2, -roll);
    SkyRenderer::MatrixMultiply(tmp, pitch, rollM);
    SkyRenderer::MatrixMultiply(out, yaw, tmp);
    return true;
}

// ---- 13.19 fixed point ----

// @0048d910 float to sign-and-magnitude style fixed point: integer part in the top 13 bits, 19 fraction bits, negated when
// the value is negative. The valid range is +-4096; outside it the game's error reporter is called and the result is junk.
static unsigned ToFixed(float v)
{
    if (FloatBits(v) > 0x45800000 || (double)v < -4096.0)
        NumericError();
    float a = v;
    if ((double)v < 0.0)
        a = -a;
    int whole = (int)a;
    int frac = (int)(((long double)a - whole) * 524288.0);
    unsigned r = (unsigned)frac | ((unsigned)whole << 19);
    if ((double)v < 0.0)
        r = -r;
    return r;
}

// ---- library entry points ----

// @0048bb30
unsigned char SkyRenderer::Init(int r, int c, unsigned char flag)
{
    if (r < 1)
        return 2;
    rows = r > MAX_ROWS ? MAX_ROWS : r;      // not in the original, which would overrun its tables: only the top rows are drawn
    cols = c;
    colsSquared = c * c;
    fovX = 90.0f;
    fovY = 60.0f;
    yUp = flag;
    BuildRays();
    initialized = true;
    return 0;
}

// @0048bba0
unsigned char SkyRenderer::Shutdown()
{
    if (!initialized)
        return 1;
    initialized = false;
    return 0;
}

// @0048bbc0
unsigned char SkyRenderer::SetFace(const void *pixels, const WarpFace *f)
{
    if (!initialized)
        return 1;

    // the face's own axis: (0, 0, 1) for the cube faces. faceMatrix is the transpose of the rotation that takes it to +z.
    float axis[3] = { f->zero08, f->zero0c, f->one };
    float rot[9] = {};
    AxisRotation(axis, 0.0f, rot);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            faceMatrix[i * 3 + j] = rot[j * 3 + i];
    faceAxisNeg[0] = -axis[0];
    faceAxisNeg[1] = -axis[1];
    faceAxisNeg[2] = -axis[2];

    if (f->extentA > 0x1000)
        return 3;
    face = (const unsigned char *)pixels;
    faceHeight = f->extentA;
    faceWidth = f->extentB;
    centerX = f->halfA;
    centerY = f->halfB;
    radius = f->halfA2;
    lensLimit = f->k20;
    lensA = f->zero24;
    lensB = f->k28;
    wrap = f->flag;
    if (wrap)
        return 0;

    // A lens that does not fit in the image, or a limit above 1, is refused (after everything has been stored, as in the
    // original). The centre and the limit must not be negative (-0 passes), the image must be larger than the centre.
    if ((unsigned)FloatBits(f->halfA) > 0x80000000u) return 2;
    if (!((long double)f->extentB > f->halfA)) return 2;
    if ((unsigned)FloatBits(f->halfB) > 0x80000000u) return 2;
    if (!((long double)f->extentA > f->halfB)) return 2;
    if ((unsigned)FloatBits(f->k20) > 0x80000000u) return 2;
    if (1.0 < (double)f->k20 || f->k20 != f->k20) return 2;
    return 0;
}

// @0048bd30
unsigned char SkyRenderer::SetOutput(void *bits, int d)
{
    if (!initialized)
        return 1;
    if (d != 8 && d != 16)
        return 2;
    depth = d;
    output = bits;
    return 0;
}

// @0048ca50
// The rays of every output row. Horizontally the three rays of a row are at -fovX/2, 0 and +fovX/2; vertically the rows
// sweep fovY. Each ray is x, sin(row angle), cos(fovX/2), then normalised. (The sin of the row angle is not a tangent, so
// this is not a plain pinhole camera.)
void SkyRenderer::BuildRays()
{
    long double halfX = (long double)fovX * kHalfDegToRad;
    float sinX = (float)(double)sinl(halfX);
    float cosX = (float)cosl(halfX);
    float start = (float)((long double)fovY * -kHalfDegToRad);
    float step = (float)(((long double)fovY / (rows - 1)) * kDegToRad);
    if (yUp == 0) {
        start = -start;
        step = -step;
    }
    float cosX2 = cosX * cosX;
    long double angle = start;                       // accumulated in extended precision
    for (int r = 0; r < rows; r++) {
        float sinY = (float)sinl(angle);
        angle += step;
        const float xs[3] = { -sinX, 0.0f, sinX };
        for (int k = 0; k < 3; k++) {
            float *v = rays[r][k];
            v[0] = xs[k];
            v[1] = sinY;
            v[2] = cosX;
            long double len = sqrtl((long double)v[1] * v[1] + (long double)v[0] * v[0] + cosX2);
            v[0] = (float)((long double)v[0] / len);
            v[1] = (float)((long double)v[1] / len);
            v[2] = (float)((long double)v[2] / len);
        }
    }
}

// @0048d050
// Where a unit direction (x, y, z) lands in the face image: the angle from the face axis (z) is shaped by the lens curve
// into a distance from the lens centre, the direction around the axis (atan2 of -y and x) gives the bearing.
// Note the caller passes -y as the second argument.
void SkyRenderer::Project(float x, float negY, float z, float *outX, float *outY) const
{
    long double angle = acosl((long double)z) * kInvPi;
    float t = (float)angle;
    if (!(angle >= lensLimit))
        t = (float)(((long double)lensA * t + lensB) * t);
    else
        t = 1.0f;
    float r = (float)((long double)radius * t);

    if (FloatBits(x) >= 0x3727c5ac || (unsigned)FloatBits(x) >= 0xb727c5acu) {
        long double bearing = atan2l((long double)negY, (long double)x);
        *outX = (float)(cosl(bearing) * r);
        *outY = (float)(sinl(bearing) * r);
    } else {                                         // |x| < 1e-5: straight up or down in the face
        *outX = 0.0f;
        *outY = FloatBits(negY) > 0 ? r : -r;
    }
    *outX += centerX;
    *outY += centerY;
}

// @0048bd60
// Takes the view matrix, rejects it unless it is a rotation and the face covers it, then projects the three rays of every
// row into the face image (faceX/faceY). The matrix is only remembered when everything worked.
unsigned char SkyRenderer::SetView(const float *m)
{
    if (!initialized)
        return 1;

    float mt[9], g[9];
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            mt[i * 3 + j] = m[j * 3 + i];
    MatrixMultiply(g, m, mt);                        // m * m^T must be the identity (within 0.005)
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            float v = g[i * 3 + j];
            if (i == j) {
                long double d = (long double)v - 1.0;
                if (!(d < 0.005 && d > -0.005))
                    return 3;
            } else if (!(v < 0.005f && v > -0.005f)) {
                return 3;
            }
        }

    if (!wrap) {
        // a single (non wrapping) face: the camera has to point far enough away from the face's axis
        float lenCol = (float)sqrtl((long double)m[5] * m[5] + (long double)m[2] * m[2] + (long double)m[8] * m[8]);
        long double lenAxis = sqrtl((long double)faceAxisNeg[2] * faceAxisNeg[2] + (long double)faceAxisNeg[1] * faceAxisNeg[1] +
                                    (long double)faceAxisNeg[0] * faceAxisNeg[0]);
        long double dot = (long double)m[8] * faceAxisNeg[2] + (long double)m[5] * faceAxisNeg[1] + (long double)m[2] * faceAxisNeg[0];
        double angle = (double)acosl(dot / (lenAxis * lenCol));
        if ((long double)fovY * kHalfDegToRadD > angle)
            return 2;
        if ((long double)fovX * kHalfDegToRadD > angle)
            return 2;
    }

    if (mode != 1)
        return 2;                                    // the other modes are not reconstructed (the game never selects them)

    float local[9];
    MatrixMultiply(local, faceMatrix, m);

    // the corner and middle rays must all land inside the lens circle
    float radius2 = radius * radius;
    for (int k = 0; k < 3; k++) {
        const float *probes[3] = { rays[0][k], rays[rows / 2][k], rays[rows - 1][k] };
        for (int p = 0; p < 3; p++) {
            float v[3], px, py;
            MulVector(v, local, probes[p]);
            Project(v[0], -v[1], v[2], &px, &py);
            float d2 = (px - centerX) * (px - centerX) + (py - centerY) * (py - centerY);
            if (radius2 < d2 || radius2 == d2)
                return 2;
        }
    }

    for (int r = 0; r < rows; r++)
        for (int k = 0; k < 3; k++) {
            float v[3];
            MulVector(v, local, rays[r][k]);
            Project(v[0], -v[1], v[2], &faceX[r][k], &faceY[r][k]);
        }
    memcpy(view, m, sizeof view);
    return 0;
}

// @0048c620
unsigned char SkyRenderer::SetOrigin(float x, float y, float z)
{
    if (!initialized)
        return 1;
    float angles[3] = { x * kDegToRad, y * kDegToRad, z * kDegToRad };
    float m[9];
    EulerMatrix(angles, m);
    return SetView(m);
}

// @0048c680
unsigned char SkyRenderer::SetFov(float fx, float fy)
{
    if (!initialized)
        return 1;
    if (FloatBits(fx) >= 0x43340000 || !(0.0 < (double)fx))
        return 2;
    if (FloatBits(fy) >= 0x43340000 || !(0.0 < (double)fy))
        return 2;
    fovX = fx;
    fovY = fy;
    BuildRays();
    SetView(view);                                   // project again with the new rays
    return 0;
}

// @0048c830
unsigned char SkyRenderer::Resize(int r, int c)
{
    if (!initialized)
        return 1;
    if (r < 1)
        return 2;
    rows = r > MAX_ROWS ? MAX_ROWS : r;      // see Init
    colsSquared = c * c;
    cols = c;
    return 0;
}

// @0048c870
unsigned char SkyRenderer::SetMode(char m)
{
    if (m != 0 && m != 1 && m != 2)
        return 1;
    mode = m;
    SetView(view);
    return 0;
}

// @0048ddc0 (8 bit) and @0048e230 (16 bit), the same code with a different texel size.
// Fills one output row. px and py hold the face-image position of the row's left end, middle and right end. A parabola
// through those three points, a + b*c + 2*d*c*c for c = 0..cols, is walked with forward differences in 13.19 fixed point.
template <typename Pixel>
void SkyRenderer::DrawRow(Pixel *dst, float *px, float *py)
{
    if (wrap) {
        // keep the three x values in one lap of the image: a row that crosses the seam is unrolled
        if (px[0] > px[1] && px[1] < px[2])
            px[2] = px[2] - faceWidth;
        else if (px[0] < px[1] && px[2] < px[1])
            px[0] = faceWidth + px[0];
    }

    long double w = cols, w2 = colsSquared;
    long double bx = (((long double)px[0] * -3.0f - px[2]) + (long double)px[1] * 4.0f) / w;            // linear term
    float       dx2 = (float)((((long double)px[1] * -2.0f + px[0]) + px[2]) / w2 * 2.0f);            // 2 * quadratic term
    float       by = (float)((((long double)py[0] * -3.0f - py[2]) + (long double)py[1] * 4.0f) / w);
    float       dy2 = (float)((((long double)py[1] * -2.0f + py[0]) + py[2]) / w2 * 2.0f);

    unsigned stepX = ToFixed((float)(bx + dx2));       // first difference
    unsigned accX = ToFixed(dx2 * 2.0f);               // second difference
    unsigned stepY = ToFixed((float)((long double)dy2 + by));
    unsigned accY = ToFixed(dy2 * 2.0f);
    unsigned x = ToFixed(px[0]);
    unsigned y = ToFixed(py[0]);

    const Pixel *src = (const Pixel *)face;
    for (Pixel *end = dst + cols; dst < end; dst++) {
        int ix = (int)x >> 19;
        int iy = (int)y >> 19;
        x += stepX;
        stepX += accX;
        y += stepY;
        stepY += accY;
        *dst = src[iy * faceWidth + ix];
    }
}

// @0048c710
// Rows are written "cols" pixels apart, ignoring the surface's pitch, exactly as the original does: it works while the warp
// surface's pitch equals its width (it has so far), and would shear the sky for a width that is not a multiple of 4 (8 bit) or 2 (16 bit).
unsigned char SkyRenderer::Draw()
{
    if (!initialized)
        return 1;
    if (face == NULL)
        return 2;
    if (output == NULL)
        return 4;
    if (mode != 1)
        return 0;                                    // not reconstructed, see SetView
    if (depth == 16) {
        for (int r = 0; r < rows; r++)
            DrawRow((unsigned short *)output + cols * r, faceX[r], faceY[r]);
    } else {
        for (int r = 0; r < rows; r++)
            DrawRow((unsigned char *)output + cols * r, faceX[r], faceY[r]);
    }
    return 0;
}
