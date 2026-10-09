// Menu: the game's main menu. Only the trivial leaves are ported here.
#pragma once
#include "compat41.h"

struct Menu {
    void *vtbl;       // 0x000
    int   itemCount;  // 0x004
    int   left;       // 0x008
    int   top;        // 0x00c
    int   itemHeight; // 0x010
    int   spacing;    // 0x014
    char  items[0x24 * 10];  // 0x018
    int   anchorX;    // 0x180
    int   anchorY;    // 0x184
    int   arg2;       // 0x188
    int   arg1;       // 0x18c

    void SaveAnchor();         // 0x406b80
    static int ActionStart();   // 0x406950, returns 2
};
