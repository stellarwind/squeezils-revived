// Unnamed helpers in the 0x459xxx region (a display/track config object), VC++ 4.1 dialect.
// The object starts with a short resolution pair and has int fields further out (0x14/0x18/0x1c seen in 0x459320/0x4593c0).
#pragma once
#include "compat41.h"

struct DisplayConfig {
    short w, h;                 // 0x00 (resolution, read by ToInt)
    char  pad[0x18];            // 0x04 .. 0x1c
    int   field1c;              // 0x1c

    void ToInt(int *dst);       // 0x459130 sign-extend (w,h) into (int,int)
    int  GetField1c();          // 0x4593c0
};
