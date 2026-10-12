// Registers an object's name in the global name list (SQZ.EXE 0x421d00, cdecl, 71 callers).
#include "compat41.h"
#include "String/String.h"

struct Named {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual const char *NameOf(Named **self, int n);     // slot 4 (+0x10)
};
struct NameList { void Add(const String &s); };           // 0x46c330
extern NameList g_names;                                   // 0x4c70a0

void RegisterName(Named *n)
{
    g_names.Add(String(n->NameOf(&n, -1)));
}
