// Warp members, SQZ.EXE 0x478c80-0x478fd0 (Load 0x479090 is in WarpLoad.cpp).
#include <string.h>
#include <math.h>
#include "Warp/Warp.h"

Warp::Warp()
{
    fovX = 90.0f;
    fovY = 60.0f;
    faceCount = 0;
    memset(faceData, 0, sizeof(faceData));
    current = 0;
}

Warp::~Warp()
{
    SkyShutdown();
    for (int i = 0; i < 6; i++)
        if (faceData[i])
            Lib_Free(faceData[i]);
}

int Warp::Init(int h, int w, float fov, char flag)
{
    height = h;
    width = w;
    int r = SkyInit(h, w, 0);
    if (r)
        return 0;
    SkySetMode(flag);
    SkySetOrigin(0.0f, 0.0f, 0.0f);
    fovX = fov;
    fovY = (float)h / (float)w * fov;
    SkySetFov(fovX, fovY);
    if (faceData[current] == 0)
        return 0;
    SkySetFace(faceData[0], &faces[0]);
    return 1;
}

void Warp::SetOutput(void *bits)
{
    SkySetOutput(bits, outputDepth);
}

void Warp::Resize(int w, int h)
{
    SkyResize(h, w);
    fovY = (float)h / (float)w * fovX;
    SkySetFov(fovX, fovY);
}

void Warp::Update(const float *m)
{
    if (faceCount > 1)
        UpdateCube(m);
    else
        UpdateSingle(m);
}

void Warp::UpdateSingle(const float *m)
{
    float m3[9];
    m3[0] = m[0];
    m3[3] = m[1];
    m3[6] = m[2];
    m3[1] = m[4];
    m3[4] = m[5];
    m3[7] = m[6];
    m3[2] = m[8];
    m3[5] = m[9];
    m3[8] = m[10];
    SkySetView(m3);
}

void Warp::UpdateCube(const float *m)
{
    float m3[9];
    m3[0] = m[0];
    m3[3] = m[1];
    m3[6] = m[2];
    m3[1] = m[4];
    m3[4] = m[5];
    m3[7] = m[6];
    m3[2] = m[8];
    m3[5] = m[9];
    m3[8] = m[10];
    if (faceCount == 0)
        return;
    int face = SelectFace(m3);
    SkySetFace(faceData[face], &faces[face]);
    float rot[9];
    switch (face) {
    case 0:
        RotationMatrix(rot, 1, 3.1415927f);
        break;
    case 1:
        RotationMatrix(rot, 0, 1.5707964f);
        break;
    case 2:
        {
            int i;
            for (i = 0; i < 3; i++) {
                rot[i * 3] = 0.0f;
                rot[i * 3 + 1] = 0.0f;
                rot[i * 3 + 2] = 0.0f;
            }
        }
        rot[8] = 1.0f;
        rot[4] = 1.0f;
        rot[0] = 1.0f;
        break;
    case 3:
        RotationMatrix(rot, 1, -1.5707964f);
        break;
    case 4:
        RotationMatrix(rot, 1, 1.5707964f);
        break;
    case 5:
        RotationMatrix(rot, 0, -1.5707964f);
        break;
    }
    float out[9];
    MatrixMultiply(out, rot, m3);
    SkySetView(out);
}

int Warp::SelectFace(const float *m3)
{
    float ax = (float)fabs(m3[2]);
    float ay = (float)fabs(m3[5]);
    float az = (float)fabs(m3[8]);
    int axis;
    if (ax > ay) {
        if (az < ax)
            axis = 0;
        else
            axis = 2;
    } else {
        if (az < ay)
            axis = 1;
        else
            axis = 2;
    }
    switch (axis) {
    case 0:
        if (*(int *)&m3[2] > 0)
            return 4;
        return 3;
    case 1:
        if (*(int *)&m3[5] > 0)
            return 5;
        return 1;
    case 2:
        if (*(unsigned *)&m3[8] > 0x80000000)
            return 0;
        return 2;
    }
    return *(int *)&ax;
}
