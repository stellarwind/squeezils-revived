// Renderer: DirectDraw/Direct3D setup. Only the trivial pixel-format copy is ported here.
#pragma once
#include "compat41.h"

struct Renderer {
    char pad[0x38];
    int  format[6];       // 0x38 (DisplayFormat: bpp, isPalettized, r/g/b/alpha masks)
    char pad2a[0x58 - 0x50];
    int  screenRect[4];   // 0x58 (RECT: left, top, right, bottom)
    char pad2b[0x9ac - 0x68];
    int  textureColors;   // 0x9ac
    int  textureShades;   // 0x9b0
    int  shades;          // 0x9b4
    char pad3[0x9bc - 0x9b8];
    float nearPlane;      // 0x9bc
    float farPlane;       // 0x9c0
    float fovDegrees;     // 0x9c4

    int *GetPixelFormat(int out[6]);   // 0x475000 / 0x475020 (identical)
    void SetShades(int shades);        // 0x475040
    void SetTextureColors(int colors); // 0x475060
    void SetTextureShades(int shades); // 0x475080
    void SetViewParams(float nearPlane, float farPlane, float fovDegrees);  // 0x474f40
    void GetScreenRect(int out[4]);    // 0x474fe0
    void Rebuild();                    // dummy (the real 0x4756a0/0x4757d0)
};
