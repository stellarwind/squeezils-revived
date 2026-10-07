// ListenerTable: fixed registry of 16 (callback, context) pairs, stored at offset 0 of the game's window object.
// Reconstructed from SQZ.EXE 0x477ef0, 0x477f20, 0x477f60, 0x478130. Used by the renderer and other window code.
#pragma once
#include <windows.h>

struct ListenerTable {
    enum { CAPACITY = 16 };
    struct Entry { void *callback; void *context; };
    Entry entry[CAPACITY];

    int  FindFreeSlot() const;                    // @478130 first entry with callback == NULL, or -1
    BOOL Contains(void *callback, void *context) const;   // @477f20
    BOOL Add(void *callback, void *context);      // @477ef0 returns FALSE when the table is full
    void Remove(void *callback, void *context);  // @477f60 clears the entry and shifts later ones down (the last slot keeps a stale copy of its old contents)
};
static_assert(sizeof(ListenerTable) == 0x80, "ListenerTable layout");
