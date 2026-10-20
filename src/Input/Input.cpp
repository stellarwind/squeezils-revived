// AppInput getters/forwarders (0x43aee0, 0x43c1a0, 0x43c7a0).
#include "Input/Input.h"

int AppInput::InputJoystickMode()  { return joystickMode; }
int AppInput::ApplicationRunning() { return running; }
int AppInput::InputJoystickPresent() { return joystick->Present(); }
