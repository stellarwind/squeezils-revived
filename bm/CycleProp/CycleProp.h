// CycleProp: SetCurrent's thin forwarding wrapper (0x429370).
#pragma once
#include "compat41.h"

struct CycleProp {
    void SetCurrent(int child);      // 0x428fd0
    void SetCurrentThin(int child);  // 0x429370
};
