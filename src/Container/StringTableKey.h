// Shared prefix view of String-key tables; the complete table owns more fields.
#pragma once
#include "String/String.h"

struct KeySet {
    virtual String Get(int index); // vtable+0x00, hidden String return buffer
};
struct StringTable {
    int vtbl; // 0x00
    KeySet keys; // 0x04
    String KeyAt(int index); // 0x41bb80/0x41d800, Fam112/KeyGet.cpp
};
typedef char StringTableKeyPrefixSize[(sizeof(StringTable) == 0x08) ? 1 : -1];

inline String AnimationTableKeyAt(void *table, int index)
{
    return ((StringTable *)table)->KeyAt(index);
}
