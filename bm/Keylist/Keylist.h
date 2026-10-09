// Keylist (SQZ.EXE vtable 0x49e314): three simple accessor leaves.
// field_4 is a pointer to a 20-byte element array; slots 6/7 return it, slot 5 indexes into it.
#pragma once
#include "compat41.h"

struct Keylist {
    void *vtbl;      // +0
    char *field_4;   // +4 (20-byte element array)

    void *Slot5(int i);   // return field_4 + i*20   (0x43d620)
    void *Slot6(int);     // return field_4          (0x43d610)
    void *Slot7(int);     // return field_4          (0x43d600)
};
