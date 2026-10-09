// PossessionsTable (Table<int, Possessions*, ...>) — the out-of-line Get (0x465340).
#pragma once
#include "compat41.h"

struct SetInt5 {
    virtual int Get(int i);              // slot 0
    virtual int Insert(const int &v, int at); // slot 1
    virtual void RemoveAt(int i);        // slot 2
    virtual void Replace(int i, const int &v); // slot 3
    virtual int *Data();                 // slot 4
    virtual int Grow(int extra);         // slot 5
    virtual int Contains(const int &v);  // slot 6
    virtual int IndexOf(int v);          // slot 7
    virtual void RemoveKey(const int &v); // slot 8
    int count;
    int capacity;
    int *data;
    char bufferVtbl[4];
};

struct PossessionsTable {
    virtual int Insert(const int &k, void *const &v, int hint);
    virtual int Contains(const int &k);
    virtual int IndexOf(const int &k);
    virtual void RemoveAt(int i);
    virtual void RemoveKey(const int &k);
    SetInt5 keys;         // 0x04
    char values_pad[0xc]; // 0x18..0x24
    void **data;          // 0x24

    void *Get(int id);    // 0x465340
};
