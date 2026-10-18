// AppInput::RevertBindings, SQZ.EXE 0x43ad30.
#include "Input/Input.h"

int AppInput::RevertBindings(int which)
{
    if (which == 2) {
        for (int index = 0; index < 51; ++index) {
            bindings->ResetCommandDefault(index);
            InputBindings *commands = bindings;
            int value = commands->ReadCommandDefault(index);
            commands->SetCommandValue(index, value);
        }
    }
    if (which == 1)
        bindings->LoadStoredBindings();
    return 1;
}
