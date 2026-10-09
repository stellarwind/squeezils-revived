// DisplayObj: a polymorphic object in the 0x459xxx region (vtable at 0x0, three float fields at 0x210/0x218/0x21c).
// The getters/setters below are non-virtual leaf methods; 0x459000 is a compiler-generated virtual-dispatch thunk (slot 0x3c).
#pragma once
#include "compat41.h"

struct DisplayObj {
    void *vtbl;                  // 0x00
    char  pad1[0x210 - 0x4];     // 0x04 .. 0x210
    float f210;                  // 0x210
    char  pad2[0x218 - 0x214];   // 0x214 .. 0x218
    float f218;                  // 0x218
    float f21c;                  // 0x21c

    void  SetF21c(float v);      // 0x459010
    float GetF21c();             // 0x459020
    float GetF210();             // 0x459030
    float GetF218();             // 0x459040
};
