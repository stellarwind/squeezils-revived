// Table<String, IntList*, ...> (InventoryTable) — the out-of-line Get (0x466210). Byte-identical to the int-keyed Gets.
#pragma once
#include "compat41.h"

struct SetStr {
    virtual int Get(int i);              // slot 0
    virtual int Insert(const void *const &v, int at); // slot 1
    virtual void RemoveAt(int i);        // slot 2
    virtual void Replace(int i, const void *const &v); // slot 3
    virtual void **Data();               // slot 4
    virtual int Grow(int extra);         // slot 5
    virtual int Contains(const void *const &v); // slot 6
    virtual int IndexOf(const void *const &v);   // slot 7
    virtual void RemoveKey(const void *const &v); // slot 8
    int count;
    int capacity;
    void **data;
    char bufferVtbl[4];
};

struct StringTable {
    virtual int Insert(const void *const &k, void *const &v, int hint);
    virtual int Contains(const void *const &k);
    virtual int IndexOf(const void *const &k);
    virtual void RemoveAt(int i);
    virtual void RemoveKey(const void *const &k);
    SetStr keys;         // 0x04
    char values_pad[0xc]; // 0x18..0x24
    void **data;         // 0x24

    void *Get(const void *const &k); // 0x466210
};
