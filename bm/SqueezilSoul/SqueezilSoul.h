// SqueezilSoul: the soul of a squeezil. Only four trivial getters are ported (0x404fc0-0x405010, 0x450300).
// Offsets from Repo/src/SqueezilSoul.h.
#pragma once
#include "compat41.h"

struct SqueezilPhysics {
    char  pad294[0x294];
    float speed294;     // 0x294
    float speed298;     // 0x298
};

struct SqueezilSoul {
    char pad3c[0x3c];
    int  state;         // 0x3c
    char pad44[0x44 - 0x40];
    int  frozen;        // 0x44
    SqueezilPhysics *physics;  // 0x48
    char pad6c[0x6c - 0x4c];
    void *possessions;  // 0x6c
    void *place;        // 0x70
    void *body;         // 0x74

    void *GetInterface(int which); // 0x4503f0 slot 1
    int  GroundStateForSpeed(float speed);   // 0x456590 slot 27
    int  GetState();        // 0x405010 slot 25
    int  IsDying();         // 0x405020 slot 26 (dying or dead)
    int  CanEnter(int newState);   // 0x404ff0 slot 23
    int  IsActive();        // 0x450410 slot 2 (not dead and not frozen)
    void Freeze();          // 0x456990 slot 15 (frozen++)
    void Unfreeze();        // 0x4569a0 slot 16 (frozen--, clamped at 0)
    void *GetPhysics();     // 0x404fd0 slot 10
    void *GetBody();        // 0x404fc0 slot 17
    void *GetPossessions(); // 0x450300 slot 18
    void *TakePossessions(); // 0x450310 slot 19 (hands them over: clears the field)
    void *GetDefaultPhysics(); // 0x404fe0 slot 20 (returns the global at 0x4c7ad0)
};
