// Primitive-array class-name builders (SQZ.EXE 0x4604c0..0x466840): copy the shared base name once, then stamp the element code.
#define BUFFEROBJECT_SIGNATURES
#include "Container2/ObjectArrayViews.h"
#include <stdio.h>
#include <string.h>
extern const char *g_arrayBase;     // 0x4bf958
extern char *g_arrayName;           // 0x4bf95c
#define PRIM_NAME(fn, code) \
    const char *fn() \
    { \
        char *s = g_arrayName; \
        if (!s) { \
            s = new char[strlen(g_arrayBase) + 1]; \
            strcpy(s, g_arrayBase); \
        } \
        g_arrayName = s; \
        sprintf(s + strlen("PrimArray"), "%2x", code); \
        return g_arrayName; \
    }
PRIM_NAME(ObjArrayC::TypeName, 0x5)
PRIM_NAME(ObjArrayD::TypeName, 0xa)
