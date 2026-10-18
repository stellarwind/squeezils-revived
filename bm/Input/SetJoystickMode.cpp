// AppInput::SetJoystickMode, SQZ.EXE 0x43aef0.
#include "Input/Input.h"

int AppInput::SetJoystickMode(int mode)
{
    joystickMode = mode;
    if (mode == 1)
        mouse->ShowCursor(1);
    if (mode == 2) {
        mouse->ShowCursor(0);
        joystick->Capture(1);
    }
    return 1;
}
