// Behavior: what a robo squeezil does.
#pragma once
#include "compat41.h"

// SIZE 0x0c
struct Behavior {
    virtual ~Behavior();                        // vtable+0x00, complete 0x403b50
    virtual int Execute(int dt, int unused) = 0; // vtable+0x04
    void  *body;    // 0x04
    void  *soul;    // 0x08

    Behavior(void *body, void *soul);   // 0x4022f0
};
