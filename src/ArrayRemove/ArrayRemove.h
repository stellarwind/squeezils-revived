// Array<T>::Remove (shift-down loop) - byte-identical across every 4-byte-POD instantiation.
#pragma once
#include "compat41.h"

struct ArrayRemove {
    int vtbl_pad;   // +0
    int count;      // +4
    int capacity;   // +8
    int *data;      // +0xc

    void Remove(int i);
};
