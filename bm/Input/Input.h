// AppInput: the game's application/input object. Three trivial getters/forwarders are ported.
#pragma once
#include "compat41.h"

struct Joystick {
    int Present();   // 0x448060 (thiscall, out-of-line)
    int Capture(int on); // 0x4479d0
};
struct InputMouse { void ShowCursor(int show); }; // 0x4592f0
struct InputBindings {
    char pad0[0xd98];
    int maxCommandValue; // 0xd98
    int ResetCommandDefault(int index); // 0x43ebc0
    int ReadCommandDefault(int index);  // 0x43dcd0
    int SetCommandValue(int index, int value); // 0x43d660
    int LoadStoredBindings(); // 0x43ec70
    int ReadStoredCommand(int index); // 0x43de80
    int SelectCommandValue(int index, int current, int action); // 0x43d750
};

struct AppInput {
    char pad0[4];
    InputBindings *bindings; // 0x04
    InputMouse *mouse; // 0x08
    char pad0c[4];
    Joystick *joystick;        // 0x10 (the joystick/calibration object)
    char pad14[0x98 - 0x14];
    int  joystickMode;         // 0x98
    int  running;              // 0x9c

    int InputJoystickMode();    // 0x43aee0
    int ApplicationRunning();   // 0x43c1a0
    int InputJoystickPresent(); // 0x43c7a0
    int SetJoystickMode(int mode); // 0x43aef0
    int RevertBindings(int which); // 0x43ad30
};
