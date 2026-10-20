// Isolated Simulation struct to reproduce AddId (0x414180): the id list's Insert.
// Separate TU on purpose — adding these fields to Simulation.h perturbs the scan functions' registers.
#pragma once
#include "compat41.h"

struct ArrayInt {
    virtual int Get(int i);              // slot 0
    virtual int Insert(const int &v, int at); // slot 1
    virtual void RemoveAt(int i);        // slot 2
    virtual void Replace(int i, const int &v); // slot 3
    virtual int *Data();                 // slot 4
    virtual int Grow(int extra);         // slot 5
    int count;       // 0x04
    int capacity;    // 0x08
    int *data;       // 0x0c
};

struct Simulation {
    void *vtbl;       // 0x00
    void *creatures;  // 0x04
    int lastId;       // 0x08
    ArrayInt ids;     // 0x0c

    void AddId(int id); // 0x414180
};
