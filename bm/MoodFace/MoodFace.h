// MoodFace: the Squeezil's animated face in the game panel. Only the ctor is ported here.
#pragma once
#include "compat41.h"

struct MoodFace {
    struct Animation {
        void *frames[10];   // 0x00
        int   frameCount;   // 0x28
        char  name[20];     // 0x2c
    };

    Animation animation[10];  // 0x000
    int    unused280;         // 0x280
    int    x;                 // 0x284
    int    y;                 // 0x288
    char   pad28c[0x14];      // 0x28c
    void  *canvas;            // 0x2a0
    void  *background;        // 0x2a4
    int    width;             // 0x2a8
    int    height;            // 0x2ac
    int    lastMood;          // 0x2b0
    int    animationCount;    // 0x2b4
    int    frame;             // 0x2b8
    int    current;           // 0x2bc
    unsigned lastTick;        // 0x2c0
    float  frameMs;           // 0x2c4

    MoodFace();
};
