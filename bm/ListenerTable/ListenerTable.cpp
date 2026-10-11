// 25 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
void PadDecl16();
void PadDecl17();
void PadDecl18();
void PadDecl19();
void PadDecl20();
void PadDecl21();
void PadDecl22();
void PadDecl23();
void PadDecl24();
void PadDecl25();
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
