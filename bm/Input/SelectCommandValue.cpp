// InputBindings::SelectCommandValue, SQZ.EXE 0x43d750.
#include "Input/Input.h"

int InputBindings::SelectCommandValue(int index, int current, int action)
{
    if (action == 3)
        return ReadCommandDefault(index);
    if (action == 2)
        return ReadStoredCommand(index);
    int next = current + (action == 1 ? -1 : 1);
    if (next > maxCommandValue)
        next = 0;
    if (next < 0)
        next = maxCommandValue;
    return next;
}
