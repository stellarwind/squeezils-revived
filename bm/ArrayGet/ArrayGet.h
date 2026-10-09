// Array<T>::Get - the out-of-line virtual Get, byte-identical across every 4-byte-POD instantiation.
// `return data[i]` where data is at +0xc (vtbl@0, count@4, capacity@8).
#pragma once
#include "compat41.h"

struct ArrayGet {
    char pad[0xc];
    int *data;   // +0xc

    int Get(int i);
};
