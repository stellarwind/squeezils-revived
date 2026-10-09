// Simulation/LocationSimulation: creature-lookup accessors. GetCreatures (0x414150) + GetActor/GetSoul (0x414050/0x414080).
#pragma once
#include "compat41.h"

struct Creature {
    void *vtbl;    // 0x00
    void *actor;   // 0x04
    void *soul;    // 0x08
};

// Set<int,10> primary vtable stub (Array<int> + the Set slots; Contains=6, IndexOf=7, RemoveKey=8).
struct SetInt10 {
    virtual int Get(int i);              // slot 0
    virtual int Insert(const int &v, int at); // slot 1
    virtual void RemoveAt(int i);        // slot 2
    virtual void Replace(int i, const int &v); // slot 3
    virtual int *Data();                 // slot 4
    virtual int Grow(int extra);         // slot 5
    virtual int Contains(const int &v);  // slot 6
    virtual int IndexOf(const int &v);   // slot 7
    virtual void RemoveKey(const int &v); // slot 8
    int count;       // 0x04
    int capacity;    // 0x08
    int *data;       // 0x0c
    char bufferVtbl[4]; // 0x10 (BufferObject secondary vtable)
};

struct CreatureTable {
    virtual int Insert(const int &k, Creature *const &v, int hint); // slot 0
    virtual int Contains(const int &k);   // slot 1
    virtual int IndexOf(const int &k);    // slot 2
    virtual void RemoveAt(int i);         // slot 3
    virtual void RemoveKey(const int &k); // slot 4
    SetInt10 keys;         // 0x04 (0x14 bytes -> 0x18)
    char values_pad[0xc];  // 0x18..0x24 Array<Creature*> (vtbl,count,capacity)
    Creature **data;       // 0x24 values.data
};

struct Simulation {
    void *vtbl;               // 0x00
    CreatureTable *creatures; // 0x04

    void *GetCreatures();     // 0x414150
    void *GetActor(int id);   // 0x414050
    void *GetSoul(int id);    // 0x414080
    int FindIdBySoul(void *soul);  // 0x4140b0
    int FindIdByActor(void *actor); // 0x414100
};
