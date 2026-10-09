// PlaceSimulation: a place being played. Only four trivial getters are ported (0x439b50-0x439f80). Offsets from Repo/src.
#pragma once
#include "compat41.h"

struct PlaceSimulation {
    char pad38[0x38];
    void *level;       // 0x38
    void *surfaces;    // 0x3c
    char pad1f8[0x1f8 - 0x40];
    char startPlacement[4];  // 0x1f8

    void *GetLevel();          // 0x439b50
    void *GetSurfaces();       // 0x439b60
    void *StartPlacement();    // 0x439f70
    int   CanSave();           // 0x439f80 slot 9
};
