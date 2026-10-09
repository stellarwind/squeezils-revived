// Array<T>::Data (and any POD getter of a field at +0xc) - byte-identical `mov eax,[ecx+0xc]; ret`.
#pragma once
#include "compat41.h"

struct ArrayData {
    char pad[0xc];
    int *data;   // +0xc

    int *Data();
};
