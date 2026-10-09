// ListenerTable (SQZ.EXE 0x477ef0-0x478130), functions in address order.
#include "ListenerTable/ListenerTable.h"

int ListenerTable::Add(void *callback, void *context)
{
    int slot = FindFreeSlot();
    if (slot == -1)
        return 0;
    entry[slot].callback = callback;
    entry[slot].context = context;
    return 1;
}

int ListenerTable::Contains(void *callback, void *context) const
{
    for (int i = 0; i < 16; i++) {
        if (entry[i].callback == callback && entry[i].context == context)
            return 1;
    }
    return 0;
}

void ListenerTable::Remove(void *callback, void *context)
{
    int found = -1;
    int i;
    for (i = 0; i < 16; i++) {
        if (entry[i].callback == callback && entry[i].context == context) {
            found = i;
            break;
        }
    }
    if (found == -1)
        return;
    entry[found].callback = 0;
    entry[found].context = 0;
    for (i = found + 1; i < 16; i++)
        entry[i - 1] = entry[i];
}

int ListenerTable::FindFreeSlot() const
{
    for (int i = 0; i < 16; i++) {
        if (entry[i].callback == 0)
            return i;
    }
    return -1;
}
