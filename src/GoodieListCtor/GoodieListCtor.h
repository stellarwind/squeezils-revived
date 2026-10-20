// GoodieList: the row of goodie pictures in the status bar. Only the ctor (count/canvas cleared) is ported here.
#pragma once
#include "compat41.h"

struct GoodieListCtor {
    char pad[0x20];
    int  count;      // 0x20
    void *canvas;    // 0x24

    GoodieListCtor();
};
