// Warp::Load (0x479090): loads <prefix>nz.gif ... py.gif into the six faces.
#include <string.h>
#include "Warp/Warp.h"

static const char kSuffix[6][7] = { "nz.gif", "ny.gif", "pz.gif", "nx.gif", "px.gif", "py.gif" };

int Warp::Load(const char *prefix, unsigned char *palette, const int *pixelFormat)
{
    int i;
    for (i = 0; i < 6; i++) {
        if (faceData[i]) {
            Lib_Free(faceData[i]);
            faceData[i] = 0;
        }
    }
    for (i = 0; i < 6; i++) {
        char path[256];
        strcpy(path, prefix);
        strcat(path, kSuffix[i]);
        int extentA, extentB;
        if (ImageLoad(path, pixelFormat, 0, &extentB, &extentA, &faceData[i], palette) == 0)
            return 0;
        outputDepth = pixelFormat[0];
        WarpFace *f = &faces[i];
        f->extentA = extentA;
        f->extentB = extentB;
        f->zero08 = 0;
        f->zero0c = 0;
        f->one = 1.0f;
        double ha = (float)extentA * 0.5f;
        double hb = (float)extentB * 0.5f;
        f->halfA = ha;
        f->halfB = hb;
        f->halfA2 = ha;
        f->k20 = 0.6111111f;
        f->zero24 = 0;
        f->k28 = 1.6363636f;
        f->flag = 0;
        f->minusOne[5] = -1.0f;
        f->minusOne[4] = -1.0f;
        f->minusOne[3] = -1.0f;
        f->minusOne[2] = -1.0f;
        f->minusOne[1] = -1.0f;
        f->minusOne[0] = -1.0f;
    }
    void *t = faceData[4];
    faceData[4] = faceData[3];
    faceData[3] = t;
    current = 2;
    faceCount = 6;
    SkySetFace(faceData[0], &faces[0]);
    fovY = (float)height / (float)width * fovX;
    SkySetFov(fovX, fovY);
    return 1;
}
