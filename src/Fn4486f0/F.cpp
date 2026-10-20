// 0x4486f0: picks the name for an index and stores it in the target object (ret 8). Index 0x13 and empty names are rejected.
#include "String/String.h"
#include <string.h>
struct Target { char pad[0xf0]; int mode; int pad2; const char *name; void Refresh(); };   // 0x404710
struct Names { String At(int i); };                  // 0x40f310
struct Picker {
    Names names;
    int Pick(int index, Target *t);
};
int Picker::Pick(int index, Target *t)
{
    if (index == 0x13)
        return 0;
    int empty = strlen(names.At(index).c_str()) == 0;
    if (empty)
        return 0;
    if (t->mode == 6 && index == 0)
        return 0;
    t->name = names.At(index).c_str();
    t->Refresh();
    return 1;
}
