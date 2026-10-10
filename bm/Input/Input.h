// AppInput: the game's application/input object. Three trivial getters/forwarders are ported.
#pragma once
#include "compat41.h"

struct Joystick {
    int Present();   // 0x448060 (thiscall, out-of-line)
};

struct AppInput {
    char pad10[0x10];
    Joystick *joystick;        // 0x10 (the joystick/calibration object)
    char pad14[0x98 - 0x14];
    int  joystickMode;         // 0x98
    int  running;              // 0x9c

    int InputJoystickMode();    // 0x43aee0
    int ApplicationRunning();   // 0x43c1a0
    int InputJoystickPresent(); // 0x43c7a0
};
