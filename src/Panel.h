// Panel: the in-game panel (the HUD along the screen edge: nut counters, mood face, compass, messages). It is one large object,
// created by the application's start-up code (constructor 0x42d5f0, global 0x4badb0), made of the widgets reconstructed so far
// (MoodFace, NutCounter) and a lot more that is still original code. Only the fields reconstructed code uses are modelled
// here, at their real offsets (LayoutCheck.cpp asserts them); the rest of the object is padding.
#pragma once
#include "Bitmap.h"

struct Panel {
    char     before[0x428];        // 0x000 the widgets and state not reconstructed yet (nut counters at +8, the mood face, ...)
    Bitmap  *digits[10];           // 0x428 the digit pictures numb_0.bmp .. numb_9.bmp used by NutCounter
    int      pad450[4];            // 0x450
    Bitmap  *background;           // 0x460 the panel's own picture
    char     backgroundFile[20];   // 0x464 its file name, from [Interface] Panel
    int      pad478[2];            // 0x478
    char     regionScreen[1];      // 0x480 the region screen widget (original code), starts here

    BOOL LoadBitmaps();            // @42e500 read the panel picture, the region screen's pictures and the ten digits
};
static_assert(sizeof(Panel) > 0x480, "Panel prefix");
