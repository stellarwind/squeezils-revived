// RegionSimulation: one region of the map. Getters and the place table leaves are ported (0x418110-0x418160, 0x4189a0).
// Layout (offsets 0x24..0x30) from Repo/src/RegionSimulation.h. places is Table<int,PlaceDescriptor*,Array<PlaceDescriptor*>,8>.
#pragma once
#include "compat41.h"

struct PlaceTable {
    virtual int Insert(int id, void *place, int hint);   // slot 0
    virtual int Contains(const int &id);                 // slot 1 (const ref)
    char pad24[0x24];                                    // keys (0x04) + values (0x18)

    void *Get(const int &id);                            // 0x469a70 (non-virtual, const ref)
};

struct RegionSimulation {
    char pad24[0x24];
    int  region;       // 0x24
    int  location;     // 0x28
    int  placeCount;   // 0x2c
    PlaceTable places; // 0x30

    int CanSave();     // 0x4189a0 slot 9
    int PlaceCount();  // 0x418110
    int HasPlace(int id);   // 0x418120
    void *GetPlace(int id); // 0x418140
    int GetLocation(); // 0x418150
    int GetRegion();   // 0x418160
};
