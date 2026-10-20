// Possessions: what a squeezil carries. Only GetInventory (0x448990) is ported.
#pragma once
#include "compat41.h"

struct Possessions {
    void **vtbl;        // 0x00 slot 4 destroys it
    void *inventory;    // 0x04

    void *GetInventory();   // 0x448990
};
