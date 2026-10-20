// GameAlloc (0x448350) / GameFree (0x448360): thin wrappers over Lib_Malloc / Lib_Free.
#include "Alloc/Alloc.h"

extern void *Lib_Malloc(unsigned n);   // 0x4904c0
extern void  Lib_Free(void *p);        // 0x490540

void *GameAlloc(unsigned n)
{
    return Lib_Malloc(n);
}

void GameFree(void *p)
{
    Lib_Free(p);
}
