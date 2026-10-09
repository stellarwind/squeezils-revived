// Simulation/LocationSimulation: creature-lookup accessors. GetCreatures (0x414150) + GetActor/GetSoul (0x414050/0x414080).
#pragma once
#include "compat41.h"

struct Creature {
    void *vtbl;    // 0x00
    void *actor;   // 0x04
    void *soul;    // 0x08
};

struct CreatureTable {
    virtual int Insert(const int &k, Creature *const &v, int hint); // slot 0
    virtual int Contains(const int &k);   // slot 1
    virtual int IndexOf(const int &k);    // slot 2
    virtual void RemoveAt(int i);         // slot 3
    virtual void RemoveKey(const int &k); // slot 4
    char keys[0x14];          // 0x04..0x18 Set<int,10>
    char values_pad[0xc];     // 0x18..0x24 Array<Creature*> (vtbl,count,capacity)
    Creature **data;          // 0x24 values.data
};

struct Simulation {
    void *vtbl;               // 0x00
    CreatureTable *creatures; // 0x04

    void *GetCreatures();     // 0x414150
    void *GetActor(int id);   // 0x414050
    void *GetSoul(int id);    // 0x414080
};
