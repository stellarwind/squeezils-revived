// GoodieList: the row of goodie pictures in the status bar (the cherry, the nuts ... the player can carry). The settings file lists
// them in the [GoodieList] section of the interface ini: every key is a number, its value names a section, and that section's UIProp
// is the picture name (cherry -> cherry.bmp and cherry1.bmp, the second frame of a two frame animation). A sub object of the status bar
// (at +0x6c, 0x30 bytes). Reconstructed from SQZ.EXE 0x4156c0 (constructor), 0x4156d0 (destructor), 0x415750 (Init), 0x415b40 (Load)
// and 0x415d50 (Draw).
#pragma once
#include "Bitmap.h"
#include "GameString.h"

struct GoodieEntry {               // 0x34 bytes
    char    picture[20];           // 0x00 UIProp: the picture's file name without ".bmp"
    int     id;                    // 0x14 the key's number (-1 for an entry that has no UIProp)
    char    section[20];           // 0x18 the section the key points to; the goodie's name as the player's inventory gives it
    Bitmap *frame0;                // 0x2c <picture>.bmp
    Bitmap *frame1;                // 0x30 <picture>1.bmp
};
static_assert(sizeof(GoodieEntry) == 0x34, "GoodieEntry layout");

struct GoodieList {
    int          x;                // 0x00 where the list sits on the panel
    int          y;                // 0x04
    char         pad08[0x14];      // 0x08
    GoodieEntry *entries;          // 0x1c
    int          count;            // 0x20 entries in use
    Bitmap      *canvas;           // 0x24 the panel picture under the list, captured on the first draw
    int          width;            // 0x28 the largest picture
    int          height;           // 0x2c

    GoodieList();                                       // @4156c0 (only the count and canvas are cleared)
    ~GoodieList();                                      // @4156d0
    BOOL Init(int x, int y);                            // @415750 read the [GoodieList] section and make the empty pictures
    BOOL Load();                                        // @415b40 load the pictures, measure the largest
    void Draw(int frame, const String &name, Bitmap *panel);   // @415d50 put the panel picture back, then draw the goodie of that name (frame 0 or 1)
};
static_assert(sizeof(GoodieList) == 0x30, "GoodieList layout");
