// MoodFace: the Squeezil's animated face in the game panel.
#pragma once
#include "compat41.h"
class Bitmap;

struct MoodFace {
    struct Animation {
        Bitmap *frames[10]; // 0x00
        int   frameCount;   // 0x28
        char  name[20];     // 0x2c
    };

    Animation animation[10];  // 0x000
    int    unused280;         // 0x280
    int    x;                 // 0x284
    int    y;                 // 0x288
    char   pad28c[0x14];      // 0x28c
    Bitmap *canvas;           // 0x2a0
    Bitmap *background;       // 0x2a4
    int    width;             // 0x2a8
    int    height;            // 0x2ac
    int    lastMood;          // 0x2b0
    int    animationCount;    // 0x2b4
    int    frame;             // 0x2b8
    int    current;           // 0x2bc
    unsigned lastTick;        // 0x2c0
    float  frameMs;           // 0x2c4

    MoodFace();
    int Load(int px, int py);  // 0x405520
    int Draw(int force, Bitmap *panel); // 0x4059e0
    int FindAnimation(int mood); // 0x405d10
};
