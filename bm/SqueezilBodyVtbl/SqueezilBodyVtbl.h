// Isolated SqueezilBody vtable to reproduce the virtual tail-call IsOnSurface (0x459000).
// This is a separate TU from SqueezilBody.h on purpose: only the mangled name prefix matters.
#pragma once
#include "compat41.h"

struct SqueezilBody {
    virtual int v0();
    virtual int v1();
    virtual int v2();
    virtual int v3();
    virtual int v4();
    virtual int v5();
    virtual int v6();
    virtual int v7();
    virtual int IsOnSurface();   // slot 8
    virtual int v9();
    virtual int v10();
    virtual int v11();
    virtual int v12();
    virtual int v13();
    virtual int v14();
    virtual int IsOnGround();    // slot 15
};
