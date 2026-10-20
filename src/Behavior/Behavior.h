// Behavior: what a robo squeezil does. Only the base ctor (0x4022f0) is ported.
#pragma once
#include "compat41.h"

struct Behavior {
    void **vtbl;    // 0x00 (0x49d0bc in the original)
    void  *body;    // 0x04
    void  *soul;    // 0x08

    Behavior(void *body, void *soul);   // 0x4022f0
};
