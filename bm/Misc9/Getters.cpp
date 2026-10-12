// Small getters: top-of-stack readers over a (count, data) global pair (SQZ.EXE 0x423220 and three identical copies),
// a global read (0x422bd0), and the script argument evaluators (0x421150 int, 0x421440 float).
#include "compat41.h"

extern int g_count;                          // 0x4c7094
extern int *g_data;                          // 0x4c709c
int StackTop() { return g_data[g_count - 1]; }

extern int g_value;                          // 0x4bae30
int GlobalValue() { return g_value; }

struct Scope {
    int Lookup(const void *name);            // 0x461230
};
struct FloatScope {
    char pad[0x30];
    struct Sub { float Lookup(const void *name); } sub;   // 0x461ea0
};
struct IntArg {
    int pad0;
    int isConst;                             // 0x04
    int name;                                // 0x08
    int value;                               // 0x0c
    int Get(Scope *scope);
};
struct FloatArg {
    int pad0;
    int isConst;                             // 0x04
    int name;                                // 0x08
    float value;                             // 0x0c
    float Get(FloatScope *scope);
};

int IntArg::Get(Scope *scope)
{
    if (isConst)
        return value;
    return scope->Lookup(&name);
}

float FloatArg::Get(FloatScope *scope)
{
    if (isConst)
        return value;
    return scope->sub.Lookup(&name);
}
