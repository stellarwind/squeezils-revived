// Array<T>::Replace (data[i] = v) - byte-identical across every 4-byte-POD instantiation.
#pragma once
#include "compat41.h"

struct ArrayReplace {
    char pad[0xc];
    int *data;   // +0xc

    void Replace(int i, const int &v);
};
