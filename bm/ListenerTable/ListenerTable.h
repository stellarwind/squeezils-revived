// ListenerTable: fixed registry of 16 (callback, context) pairs at offset 0 of the game's window object (SQZ.EXE 0x477ef0-0x478130).
#pragma once

struct ListenerTable {
    struct Entry { void *callback; void *context; };
    Entry entry[16];

    int FindFreeSlot() const;                              // 0x478130
    int Contains(void *callback, void *context) const;     // 0x477f20
    int Add(void *callback, void *context);                // 0x477ef0
    void Remove(void *callback, void *context);            // 0x477f60
};
