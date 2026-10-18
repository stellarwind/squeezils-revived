// ReleaseB (SQZ.EXE 0x422b00): destroy every entry of the top-of-stack list, pop and delete the top
// of the object stack at 0x4c7090, then remove all script variables.
#include "compat41.h"

struct Entry { virtual void Destroy(int del); };
struct EntryList {
    int pad0, pad4;
    int count;                      // 0x08
    char pad[0x18];
    Entry **items;                  // 0x24
};
EntryList *StackTop();              // 0x423220
struct StackObj { ~StackObj(); };   // 0x46c2c0
struct ObjStack {
    int pad0;
    int count;                      // 0x04
    int pad8;
    StackObj **data;                // 0x0c
    void Remove(int i);             // 0x46c510
};
struct Vars { int pad[2]; int count; void Remove(int i); };       // 0x4611d0
extern ObjStack g_stack;            // 0x4c7090
extern Vars g_vars;                 // 0x4c7060

void ReleaseB()
{
    int off = 0;
    int i = 0;
    for (;;) {
        if (i >= StackTop()->count)
            break;
        Entry *e = *(Entry **)((char *)StackTop()->items + off);
        if (e)
            e->Destroy(1);
        off += 4;
        i++;
    }
    int n = g_stack.count;
    StackObj *top = g_stack.data[n - 1];
    if (top)
        delete top;
    g_stack.Remove(g_stack.count - 1);
    for (int j = g_vars.count - 1; j >= 0; j--)
        g_vars.Remove(j);
}
