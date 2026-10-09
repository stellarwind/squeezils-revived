// FrameProp: a group of child props under a frame. Only ChildNames (0x427f50) is ported. childNames is at +0x40.
#pragma once
#include "compat41.h"

struct FrameProp {
    char pad40[0x40];
    char childNames[4];  // 0x40 (StringList)

    void *ChildNames();  // 0x427f50 slot 12
};
