// Splash fields used by the byte-matched display functions.
#pragma once
class Bitmap;
struct Splash {
    void *vtable;
    Bitmap *picture;
    Bitmap *bar;
    int step; // 0x0c
    int maxSteps; // 0x10
    int barX, barY; // 0x14 / 0x18
    int shown; // 0x1c
    char pad20[0x5c - 0x20];
    int progressMode; // 0x5c: -1 suppresses incremental drawing
    float progress; // 0x60
    int IsShown(); // 0x44eea0
    void Reset();  // 0x44f080
    void Show();   // 0x44edd0
    void Pump();   // 0x44eeb0
    void DrawProgress(); // 0x44ef30
};
