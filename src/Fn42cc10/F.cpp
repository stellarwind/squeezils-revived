// SQZ.EXE 0x42cc10 (cdecl): look a name up in three global tables in turn and return the first match, 0 if none.
#include "String/String.h"

struct NameTable {
    int Contains(const String &name);                    // 0x468e50
    void *Get(const String &name);                       // 0x468ef0
};
extern NameTable gTableA, gTableB, gTableC;              // 0x4c71d0 0x4c7258 0x4c7280

void *LookupByName(const char *name)
{
    if (gTableA.Contains(String(name)))
        return gTableA.Get(String(name));
    if (gTableB.Contains(String(name)))
        return gTableB.Get(String(name));
    if (gTableC.Contains(String(name)))
        return gTableC.Get(String(name));
    return 0;
}
