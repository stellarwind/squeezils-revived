// String-returning accessor forwarding to a non-virtual String Get(int) of the member at +0x18
// (SQZ.EXE 0x40f2a0 and 3 identical copies).
#include "compat41.h"
#include "String/String.h"

struct StringArray { String Get(int i); };
struct Holder {
    char pad[0x18];
    StringArray items;
    String Item(int i);
};

String Holder::Item(int i)
{
    return items.Get(i);
}
