// Isolated Simulation/CreatureTable to reproduce AddCreature (0x414160): a forwarding wrapper to Put.
// Separate TU on purpose — adding this to Simulation.h perturbs the scan functions' registers.
#pragma once
#include "compat41.h"

struct CreatureTable {
    void Put(int id, void *actor, void *soul); // 0x412da0 (out-of-line)
};

struct Simulation {
    void *vtbl;               // 0x00
    CreatureTable *creatures; // 0x04

    void AddCreature(int id, void *actor, void *soul); // 0x414160
};
