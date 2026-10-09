// Prop: a placed thing with a D3DRM frame. Only IsShown (0x426160) is ported. Layout from Repo/src/Prop.h.
#pragma once
#include "compat41.h"

struct Float3 { float x, y, z; };

struct Prop {
    void  **vtbl;       // 0x00
    Float3 position;    // 0x04
    Float3 scale;       // 0x10
    Float3 direction;   // 0x1c
    Float3 up;          // 0x28
    void  *frame;       // 0x34 (NULL while hidden)
    void  *owner;       // 0x38
    int    isInstance;  // 0x3c

    int IsShown();      // 0x426160 slot 14
};
