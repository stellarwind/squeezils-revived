// LandSimulation trivial getters (0x41e810, 0x41e820).
#pragma once
#include "compat41.h"

struct PossessionsTable {
    virtual int Insert(const int &k, void *const &v, int hint); // slot 0
    virtual int Contains(const int &k);                          // slot 1
    virtual int IndexOf(const int &k);                           // slot 2
    virtual void RemoveAt(int i);                                // slot 3
    virtual void RemoveKey(const int &k);                        // slot 4
    char keysVtbl[4];   // 0x04 keys (Set<int,5>) vptr
    int  count;         // 0x08 keys.count
    char pad[0x10];     // 0x0c..0x18 (capacity, data)

    void *Get(const int &k);   // 0x465340 (out-of-line Table::Get)
};

// Set<int,10> primary vtable stub: Contains is slot 6 (offset 0x18).
struct SetInt10 {
    virtual int Get(int i);
    virtual int Insert(const int &v, int at);
    virtual void RemoveAt(int i);
    virtual void Replace(int i, const int &v);
    virtual int *Data();
    virtual int Grow(int extra);
    virtual int Contains(const int &v);
    virtual int IndexOf(const int &v);
    virtual void RemoveKey(const int &v);
};

struct LandSimulation {
    void **vtbl;              // 0x00 (Simulation)
    void **bufferVtbl;        // 0x04 (BufferObject)
    PossessionsTable *table;  // 0x08
    SetInt10 *lentIds;        // 0x0c
    char **names;             // 0x10

    int CharacterCount();              // 0x41e810 slot 7
    PossessionsTable *GetTable();      // 0x41e820 slot 8
    int IsLentOut(int id);             // 0x41e830 slot 9
    void *Lend(int id);                // 0x41e850 slot 10
    void Return(int id, void *possessions); // 0x41e880 slot 11
    const char *GetPlaceName(int index); // 0x41e8c0
};
