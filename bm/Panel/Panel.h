// Panel trivial setters (SQZ.EXE 0x42db00, 0x42ea60).
#pragma once
#include "compat41.h"

struct Panel {
    char pad45c[0x45c];
    int  dirty;              // 0x45c
    char pad460[0x614 - 0x460];
    int  event;              // 0x614

    void Invalidate();       // 0x42db00
    void ClearEvent();       // 0x42ea60
};
