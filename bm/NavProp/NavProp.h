// NavProp: a prop with navigation. Only GetSurface/GetInstance are ported (0x429690/0x4296a0). Offsets from Repo/src/NavProp.h.
#pragma once
#include "compat41.h"

struct NavProp {
    char padF4[0xf4];
    int  surface;   // 0xf4
    int  instance;  // 0xf8
    char pad12c[0x12c - 0xfc];
    float sizeSquared;  // 0x12c

    int GetSurface();   // 0x429690 slot 25
    int GetInstance();  // 0x4296a0 slot 26
    int GetSizeSquared(float *out);  // 0x429da0 slot 11
#ifdef NAVPROP_HIDE
    void Hide();        // slot 5  0x429c20
#endif
};
