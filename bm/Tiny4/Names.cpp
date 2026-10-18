// Primitive-array class-name builders (SQZ.EXE 0x4604c0..0x466840): copy the shared base name once, then stamp the element code.
#include <stdio.h>
#include <string.h>
extern const char *g_arrayBase;     // 0x4bf958
extern char *g_arrayName;           // 0x4bf95c
#define PRIM_NAME(fn, code) \
    char *fn() \
    { \
        char *s = g_arrayName; \
        if (!s) { \
            s = new char[strlen(g_arrayBase) + 1]; \
            strcpy(s, g_arrayBase); \
            g_arrayName = s; \
            sprintf(s + strlen("PrimArray"), "%2x", code); \
        } \
        return g_arrayName; \
    }
PRIM_NAME(Name4604c0, 0x14)
PRIM_NAME(Name461250, 0x15)
PRIM_NAME(Name461ec0, 0x16)
PRIM_NAME(Name462d40, 0x17)
PRIM_NAME(Name463930, 0x18)
PRIM_NAME(Name4644f0, 0x19)
PRIM_NAME(Name464ed0, 0x5)
PRIM_NAME(Name465360, 0x5)
PRIM_NAME(Name4663b0, 0xa)
PRIM_NAME(Name466840, 0xa)
