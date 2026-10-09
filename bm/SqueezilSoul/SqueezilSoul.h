// SqueezilSoul: the soul of a squeezil. Only four trivial getters are ported (0x404fc0-0x405010, 0x450300).
// Offsets from Repo/src/SqueezilSoul.h.
#pragma once
#include "compat41.h"

struct SqueezilSoul {
    char pad3c[0x3c];
    int  state;         // 0x3c
    char pad48[0x48 - 0x40];
    void *physics;      // 0x48
    char pad6c[0x6c - 0x4c];
    void *possessions;  // 0x6c
    void *place;        // 0x70
    void *body;         // 0x74

    int  GetState();        // 0x405010 slot 25
    void *GetPhysics();     // 0x404fd0 slot 10
    void *GetBody();        // 0x404fc0 slot 17
    void *GetPossessions(); // 0x450300 slot 18
    void *GetDefaultPhysics(); // 0x404fe0 slot 20 (returns the global at 0x4c7ad0)
};
