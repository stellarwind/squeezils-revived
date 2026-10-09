// CycleProp: SetCurrent's thin forwarding wrapper (0x429370).
#pragma once
#include "compat41.h"

struct CycleProp {
    char padE0[0xe0];
    int  step;   // 0xe0
    int  mode;   // 0xe4

    void SetCurrent(int child);      // 0x428fd0
    void SetCurrentThin(int child);  // 0x429370
    void SetStep(int newStep);       // 0x429350
    void SetMode(int newMode);       // 0x429360
};
