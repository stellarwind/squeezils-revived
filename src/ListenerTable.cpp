#include "ListenerTable.h"

// @00478130
int ListenerTable::FindFreeSlot() const
{
    for (int i = 0; i < CAPACITY; i++)
        if (entry[i].callback == NULL)
            return i;
    return -1;
}

// @00477f20
BOOL ListenerTable::Contains(void *callback, void *context) const
{
    for (int i = 0; i < CAPACITY; i++)
        if (entry[i].callback == callback && entry[i].context == context)
            return TRUE;
    return FALSE;
}

// @00477ef0
BOOL ListenerTable::Add(void *callback, void *context)
{
    int slot = FindFreeSlot();
    if (slot == -1)
        return FALSE;
    entry[slot].callback = callback;
    entry[slot].context = context;
    return TRUE;
}

// @00477f60
// Quirk kept from the original: after shifting the later entries down by one, the LAST slot is not cleared,
// so it keeps its old value (a stale duplicate of the entry that moved into slot 14).
void ListenerTable::Remove(void *callback, void *context)
{
    int found = -1;
    for (int i = 0; i < CAPACITY; i++) {
        if (entry[i].callback == callback && entry[i].context == context) { found = i; break; }
    }
    if (found == -1)
        return;
    entry[found].callback = NULL;
    entry[found].context = NULL;
    for (int i = found; i < CAPACITY - 1; i++)
        entry[i] = entry[i + 1];
}
