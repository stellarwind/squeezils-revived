// RegionSimulation: one region of the map. Only the four trivial getters are ported (0x418110-0x418160, 0x4189a0).
// Layout (offsets 0x24..0x2c) from Repo/src/RegionSimulation.h.
#pragma once
#include "compat41.h"

struct RegionSimulation {
    char pad24[0x24];
    int  region;       // 0x24
    int  location;     // 0x28
    int  placeCount;   // 0x2c

    int CanSave();     // 0x4189a0 slot 9
    int PlaceCount();  // 0x418110
    int GetLocation(); // 0x418150
    int GetRegion();   // 0x418160
};
