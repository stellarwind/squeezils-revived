// Simulation: the base of every simulation. Only GetCreatures (0x414150) is ported. creatures is at +4 (after the vtable).
#pragma once
#include "compat41.h"

struct Simulation {
    void *vtbl;        // 0x00
    void *creatures;   // 0x04

    void *GetCreatures();   // 0x414150
};
