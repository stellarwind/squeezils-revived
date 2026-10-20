// Input-control binding table initializer (0x43df10): copies each control's name and its
// default values/key into a descriptor array (stride 0x44) chosen by `index` (0..50).
#pragma once
#include "compat41.h"

struct PTEntry {                  // 0x44 bytes
    char  pad0[0x0c];
    float f_c;                    // +0x0c
    float f_10;                   // +0x10
    float f_14;                   // +0x14
    int   i_18;                   // +0x18
    int   i_1c;                   // +0x1c
    int   i_20;                   // +0x20
    char  name[0x20];             // +0x24
};

struct PTTail {                   // the +0x48/+0x4c ints (past the 0x44 stride)
    char pad[0x48];
    int  i_48;
    int  i_4c;
};

struct PTTable {
    PTEntry entries[1];
    int Init(int index);          // 0x43df10
};
