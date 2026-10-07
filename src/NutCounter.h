// NutCounter: a picture in the in-game panel with a number (or two) drawn on it, used for the nut counts (RTTI name not
// known; the ini keys are AmoNut and FoodNut). Reconstructed from SQZ.EXE 0x42f780-0x42fc22 (size 0x2c, sub objects of the
// status bar). The picture is loaded once from ui\<file>, a copy of it is kept, and every Draw restores the picture from that copy,
// stamps the digit pictures (the panel's ten digit bitmaps) onto it and draws it to the primary surface at its place.
#pragma once
#include "Bitmap.h"

class NutCounter {
public:
    NutCounter();                                    // @42f780
    ~NutCounter();                                   // @42f790
    BOOL Init(const char *key, int x, int y, int mode);   // @42f7d0 file name from [Interface] <key>; mode 1 shows two numbers
    BOOL Load();                                     // @42f900 load ui\<file> and keep a copy of it
    void Draw(int value, int value2);                // @42f9f0 stamp the number(s) and draw; value2 only for mode 1
    void DrawNumber(int value, int x, int y);        // @42fb10 stamp one or two digits centred on (x, y) of the picture

    Bitmap *icon;          // 0x00 the picture the numbers are stamped on
    Bitmap *original;      // 0x04 an untouched copy of it
    int     x;             // 0x08 where it sits in the panel
    int     y;             // 0x0c
    char    file[20];      // 0x10 picture file (without folder)
    int     lastValue;     // 0x24 what the last Draw showed
    int     mode;          // 0x28 1 = two numbers, one above the other; otherwise one centred
};
static_assert(sizeof(NutCounter) == 0x2c, "NutCounter layout");
