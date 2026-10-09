// LandSimulation trivial getters (0x41e810, 0x41e820).
#pragma once
#include "compat41.h"

struct PossessionsTable {
    void **vtbl;    // 0x00
    char   pad4[4]; // 0x04 keys (Set<int,5>) vptr
    int    count;   // 0x08 keys.count
};

struct LandSimulation {
    void **vtbl;              // 0x00 (Simulation)
    void **bufferVtbl;        // 0x04 (BufferObject)
    PossessionsTable *table;  // 0x08
    void  *lentIds;           // 0x0c
    char **names;             // 0x10

    int CharacterCount();              // 0x41e810 slot 7
    PossessionsTable *GetTable();      // 0x41e820 slot 8
    const char *GetPlaceName(int index); // 0x41e8c0
};
