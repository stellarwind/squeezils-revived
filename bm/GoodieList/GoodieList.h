// GoodieList: the row of goodie pictures in the status bar (SQZ.EXE 0x4156c0-0x415ee0, 0x30 bytes, no vtable), VC++ 4.1 dialect.
// Behaviour notes: Repo/src/GoodieList.h.
#pragma once
#include "Bitmap/Bitmap.h"
#include "String/String.h"

struct GoodieEntry {               // 0x34 bytes
    char picture[20];              // 0x00
    int id;                        // 0x14
    char section[20];              // 0x18
    Bitmap *frame0;                // 0x2c
    Bitmap *frame1;                // 0x30
};

struct GoodieList {
    int x;                         // 0x00
    int y;                         // 0x04
    char pad08[0x14];
    GoodieEntry *entries;          // 0x1c
    int count;                     // 0x20
    Bitmap *canvas;                // 0x24
    int width;                     // 0x28
    int height;                    // 0x2c

    GoodieList();                                       // 0x4156c0
    ~GoodieList();                                      // 0x4156d0
    int Init(int x, int y);                             // 0x415750
    int Load();                                         // 0x415b40
    void Draw(int frame, String name, Bitmap *panel);   // 0x415d50
};
