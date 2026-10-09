// StringTable::Get (0x466210).
#include "StringTable/StringTable.h"

void *StringTable::Get(const void *const &k)
{
    return data[keys.IndexOf(k)];
}
