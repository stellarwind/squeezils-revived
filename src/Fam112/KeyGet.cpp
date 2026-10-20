// The String-key accessor of the Table classes (SQZ.EXE 0x40ee50 and 8 identical copies): returns keys.Get(i) by value,
// the key set being embedded at +4 and its slot-0 virtual Get(int) returning a String through a hidden pointer.
#include "compat41.h"
#include "String/String.h"

struct KeySet { virtual String Get(int i); };
struct StringTable {
    int vtbl;
    KeySet keys;
    String KeyAt(int i);
};

String StringTable::KeyAt(int i)
{
    return keys.Get(i);
}
