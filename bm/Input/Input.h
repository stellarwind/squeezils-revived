// AppInput: the game's application/input object. Two trivial getters are ported.
#pragma once
#include "compat41.h"

struct AppInput {
    char pad98[0x98];
    int  joystickMode;      // 0x98
    int  running;           // 0x9c

    int InputJoystickMode();   // 0x43aee0
    int ApplicationRunning();  // 0x43c1a0
};
