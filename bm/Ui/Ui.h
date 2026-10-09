// Bitmap / Scene / Splash trivial getters (SQZ.EXE 0x4738b0-0x4739c0, 0x477290, 0x477390, 0x44eea0).
#pragma once
#include "compat41.h"

struct Bitmap {
    char  pad74[0x74];
    void *surface;      // 0x74
    int   width;        // 0x78
    int   height;       // 0x7c

    void *GetSurface(); // 0x4738b0
    int   GetWidth();   // 0x4739b0
    int   GetHeight();  // 0x4739c0
};

struct Renderer {
    char  pad990[0x990];
    void *cameraFrame;  // 0x990
};

struct Scene {
    Renderer *renderer; // 0x00
    char      pad0c[0xc - 0x4];
    void     *sceneChild;   // 0x0c

    void *GetSceneChild();  // 0x477290
    void *GetCameraFrame(); // 0x477390
};

struct Splash {
    char pad1c[0x1c];
    int  shown;         // 0x1c
    int  IsShown();     // 0x44eea0
};
