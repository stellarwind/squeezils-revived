// Panel trivial setters (SQZ.EXE 0x42db00, 0x42ea60, 0x42dd00, 0x42dcd0).
#pragma once
#include "compat41.h"

struct Panel {
    char pad45c[0x45c];
    int  dirty;              // 0x45c
    char pad460[0x614 - 0x460];
    int  event;              // 0x614
    char pad618[0x630 - 0x618];
    char message[100];       // 0x630
    char note[20];           // 0x694

    void Invalidate();       // 0x42db00
    void ClearEvent();       // 0x42ea60
    void ClearMessage();     // 0x42dd00
    void ClearNote();        // 0x42dcd0
};
