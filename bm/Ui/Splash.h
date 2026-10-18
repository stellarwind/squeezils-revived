// Splash fields used by the byte-matched display functions.
#pragma once
class Bitmap;
struct Splash {
    void *vtable;
    Bitmap *picture;
    Bitmap *bar;
    int step; // 0x0c
    char pad10[0x1c - 0x10];
    int shown; // 0x1c
    int IsShown(); // 0x44eea0
    void Reset();  // 0x44f080
    void Show();   // 0x44edd0
    void Pump();   // 0x44eeb0
};
