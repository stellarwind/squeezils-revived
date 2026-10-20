// NutCounter (SQZ.EXE 0x42f780-0x42fc22, 0x2c bytes, no vtable), VC++ 4.1 dialect. Behaviour: Repo/legacy_gcc/NutCounter.h.
#pragma once

#include "Bitmap/Bitmap.h"

struct Panel {
    char pad[0x428];
    Bitmap *digits[10];          // 0x428
    void GetScreenOrigin(int *x, int *y);   // 0x42da30
};

struct NutCounter {
    Bitmap *icon;          // 0x00
    Bitmap *original;      // 0x04
    int x;                 // 0x08
    int y;                 // 0x0c
    char file[20];         // 0x10
    int lastValue;         // 0x24
    int mode;              // 0x28

    NutCounter();                                           // 0x42f780
    ~NutCounter();                                          // 0x42f790
    int Init(const char *key, int x, int y, int mode);      // 0x42f7d0
    int Load();                                             // 0x42f900
    void Draw(int value, int value2);                       // 0x42f9f0
    void DrawNumber(int value, int x, int y);               // 0x42fb10
};
