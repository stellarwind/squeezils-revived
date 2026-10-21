// String-returning accessor forwarding to a non-virtual String Get(int) of the member at +0x18
// (SQZ.EXE 0x40f2a0, 0x427ae0 and 0x428af0).
// The vector forwarding accessor at 0x45d7e0 lives in Container2/GetVectorVariable.cpp.
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
