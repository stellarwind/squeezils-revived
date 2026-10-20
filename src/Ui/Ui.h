// Scene / Splash trivial getters (SQZ.EXE 0x4738b0-0x4739c0, 0x477290, 0x477390, 0x44eea0).
#pragma once
#include "compat41.h"

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

#include "Ui/Splash.h"
