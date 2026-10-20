// Table<K,V>::Contains / IndexOf / RemoveAt / RemoveKey - the out-of-line non-virtual forwarders.
// They only forward through virtual calls (no key/value-type code), so one source is byte-identical
// across every Table instantiation (int / String / pointer / float keys, any value type).
#pragma once
#include "compat41.h"

struct SetStub {
    virtual int Get(int i);                    // slot 0
    virtual int Insert(const int &v, int at);  // slot 1
    virtual void RemoveAt(int i);              // slot 2
    virtual void Replace(int i, const int &v); // slot 3
    virtual int *Data();                       // slot 4
    virtual int Grow(int extra);               // slot 5
    virtual int Contains(const int &v);        // slot 6
    virtual int IndexOf(const int &v);         // slot 7
    virtual void RemoveKey(const int &v);      // slot 8
    int count;
    int capacity;
    int *data;
    char bufferVtbl[4];
};

struct ArrayStub {
    virtual int Get(int i);                    // slot 0
    virtual int Insert(const int &v, int at);  // slot 1
    virtual void Remove(int i);                // slot 2
    virtual void Replace(int i, const int &v); // slot 3
    virtual int *Data();                       // slot 4
    virtual int Grow(int extra);               // slot 5
    int count;
    int capacity;
    int *data;
};

struct TableFwd {
    virtual int Insert(const int &k, void *const &v, int hint); // slot 0
    virtual int Contains(const int &k);   // slot 1
    virtual int IndexOf(const int &k);    // slot 2
    virtual void RemoveAt(int i);         // slot 3
    virtual void RemoveKey(const int &k); // slot 4
    SetStub keys;         // 0x04
    ArrayStub values;     // 0x18 (vtbl@0x18, count@0x1c, capacity@0x20, data@0x24)
};
