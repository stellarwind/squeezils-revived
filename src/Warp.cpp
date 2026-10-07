// Warp: see Warp.h. Reconstructed from SQZ.EXE 0x478c80-0x479090.
#include "Warp.h"
#include "Sky.h"
#include "Image.h"
#include "Original.h"
#include <math.h>
#include <string.h>


static float FloatFromBits(unsigned bits) { float f; memcpy(&f, &bits, sizeof f); return f; }

static const char *const kFaceSuffix[Warp::FACES] = { "nz.gif", "ny.gif", "pz.gif", "nx.gif", "px.gif", "py.gif" };

// @00478c80
Warp::Warp()
{
    fovX = 90.0f;
    fovY = 60.0f;
    faceCount = 0;
    memset(faceData, 0, sizeof faceData);
    current = 0;
}

// @00478cb0
Warp::~Warp()
{
    g_sky.Shutdown();
    for (int i = 0; i < FACES; i++)
        if (faceData[i])
            Lib_Free(faceData[i]);
}

// @00478ce0
int Warp::Init(int h, int w, float fov, char flag)
{
    height = h;
    width = w;
    if (g_sky.Init(h, w, 0))
        return 0;
    g_sky.SetMode(flag);
    g_sky.SetOrigin(0.0f, 0.0f, 0.0f);
    fovX = fov;
    fovY = (float)(((double)h / (double)w) * fov);
    g_sky.SetFov(fovX, fovY);
    if (faceData[current] == NULL)
        return 0;
    g_sky.SetFace(faceData[0], &faces[0]);
    return 1;
}

// @00478da0
void Warp::SetOutput(void *bits)
{
    g_sky.SetOutput(bits, outputDepth);
}

// @00478dc0
void Warp::Resize(int w, int h)
{
    g_sky.Resize(h, w);
    fovY = (float)(((double)h / (double)w) * fovX);
    g_sky.SetFov(fovX, fovY);
}

// @00478e10
void Warp::Update(const float *m)
{
    if (faceCount > 1)
        UpdateCube(m);
    else
        UpdateSingle(m);
}

// @00478e30
// One image only: pass the rotation part of m to the renderer, transposed.
void Warp::UpdateSingle(const float *m)
{
    float m3[9] = { m[0], m[4], m[8],
                    m[1], m[5], m[9],
                    m[2], m[6], m[10] };
    g_sky.SetView(m3);
}

// @00478e90
// Six images: pick the cube face the view points at, turn the view into that face's local space, draw.
void Warp::UpdateCube(const float *m)
{
    float m3[9] = { m[0], m[4], m[8],
                    m[1], m[5], m[9],
                    m[2], m[6], m[10] };
    if (faceCount == 0)
        return;

    int face = SelectFace(m3);
    g_sky.SetFace(faceData[face], &faces[face]);

    const float halfPi = 1.5707964f, pi = 3.1415927f;
    float rot[9];
    switch (face) {
    case 0: SkyRenderer::RotationMatrix(rot, 1, pi); break;
    case 1: SkyRenderer::RotationMatrix(rot, 0, halfPi); break;
    case 2:
        memset(rot, 0, sizeof rot);
        rot[0] = rot[4] = rot[8] = 1.0f;
        break;
    case 3: SkyRenderer::RotationMatrix(rot, 1, -halfPi); break;
    case 4: SkyRenderer::RotationMatrix(rot, 1, halfPi); break;
    case 5: SkyRenderer::RotationMatrix(rot, 0, -halfPi); break;
    }
    float out[9];
    SkyRenderer::MatrixMultiply(out, rot, m3);
    g_sky.SetView(out);
}

// @00478fd0
// The dominant axis of the third column picks the face; its sign picks which of the two faces on that axis.
int Warp::SelectFace(const float *m3)
{
    float ax = fabsf(m3[2]), ay = fabsf(m3[5]), az = fabsf(m3[8]);
    int axis;
    if (ax <= ay)
        axis = (az < ay) ? 1 : 2;
    else
        axis = (az < ax) ? 0 : 2;

    if (axis == 0) return m3[2] > 0.0f ? 4 : 3;
    if (axis == 1) return m3[5] > 0.0f ? 5 : 1;
    return m3[8] < 0.0f ? 0 : 2;
}

// @00479090
// prefix: path without the face suffix (the world's WarpPrefix). palette: three planes of 256 bytes (R, G, B)
// filled from the GIFs. pixelFormat: the display's DisplayFormat (bpp first).
int Warp::Load(const char *prefix, unsigned char *pal, const int *pixelFormat)
{
    for (int i = 0; i < FACES; i++)
        if (faceData[i]) {
            Lib_Free(faceData[i]);
            faceData[i] = NULL;
        }

    for (int i = 0; i < FACES; i++) {
        char path[256];
        strncpy(path, prefix, sizeof path - 1);
        path[sizeof path - 1] = 0;
        strncat(path, kFaceSuffix[i], sizeof path - strlen(path) - 1);

        int extentA, extentB;
        if (ImageLoad(path, (const DisplayFormat *)pixelFormat, NULL, &extentB, &extentA, &faceData[i], pal) == 0)
            return 0;

        outputDepth = pixelFormat[0];
        WarpFace &f = faces[i];
        f.extentA = extentA;
        f.extentB = extentB;
        f.zero08 = 0; f.zero0c = 0;
        f.one = 1.0f;
        f.halfA = (float)extentA * 0.5f;
        f.halfB = (float)extentB * 0.5f;
        f.halfA2 = (float)extentA * 0.5f;
        f.k20 = FloatFromBits(0x3f1c71c5);
        f.zero24 = 0;
        f.k28 = FloatFromBits(0x3fd1743f);
        f.flag = 0;
        for (int k = 0; k < 6; k++)
            f.minusOne[k] = -1.0f;
    }

    // the files are stored in the order nz ny pz nx px py; the renderer expects the 4th and 5th swapped
    void *t = faceData[3];
    faceData[3] = faceData[4];
    faceData[4] = t;
    current = 2;
    faceCount = FACES;
    g_sky.SetFace(faceData[0], &faces[0]);
    fovY = (float)(((double)height / (double)width) * fovX);
    g_sky.SetFov(fovX, fovY);
    return 1;
}
