// NavProp: a prop with navigation. Only GetSurface/GetInstance are ported (0x429690/0x4296a0). Offsets from Repo/src/NavProp.h.
#pragma once
#include "compat41.h"

struct NavProp {
    char padF4[0xf4];
    int  surface;   // 0xf4
    int  instance;  // 0xf8

    int GetSurface();   // 0x429690 slot 25
    int GetInstance();  // 0x4296a0 slot 26
};
