// PlaceTable (Table<int, PlaceDescriptor*, ...>) — the out-of-line Get (0x469a70).
#pragma once
#include "compat41.h"

struct SetInt8 {
    virtual int Get(int i);              // slot 0
    virtual int Insert(const int &v, int at); // slot 1
    virtual void RemoveAt(int i);        // slot 2
    virtual void Replace(int i, const int &v); // slot 3
    virtual int *Data();                 // slot 4
    virtual int Grow(int extra);         // slot 5
    virtual int Contains(const int &v);  // slot 6
    virtual int IndexOf(int v);          // slot 7 (by value)
    virtual void RemoveKey(const int &v); // slot 8
    int count;       // 0x04
    int capacity;    // 0x08
    int *data;       // 0x0c
    char bufferVtbl[4]; // 0x10
};

struct PlaceTable {
    virtual int Insert(const int &k, void *const &v, int hint); // slot 0
    virtual int Contains(const int &k);   // slot 1
    virtual int IndexOf(const int &k);    // slot 2
    virtual void RemoveAt(int i);         // slot 3
    virtual void RemoveKey(const int &k); // slot 4
    SetInt8 keys;         // 0x04
    char values_pad[0xc]; // 0x18..0x24
    void **data;          // 0x24 values.data

    void *Get(int id);    // 0x469a70
};
