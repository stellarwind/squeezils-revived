// SQZ.EXE 0x424010 (cdecl): find a resource by name (file name without directory or extension), else load it from the data directory; 0 if missing.
#include <windows.h>
#include <string.h>
#include "String/String.h"

struct Resource;
Resource *FindResource(const char *name);               // 0x425220
Resource *LoadResource(const char *a, const char *b);   // 0x425070
void DebugLog(const char *format, ...);                 // 0x419290
extern char g_dataDir[];                                // 0x4c70e8
extern const char kResExt[], kResFound[], kResMissing[];   // 0x4bb62c 0x4bb618 0x4bb5f8

Resource *OpenResource(const char *name)
{
    String text(name);
    char *p = (char *)text.c_str();
    char *base = p;
    if (!stricmp(p + strlen(p) - 4, kResExt))
        p[strlen(p) - 4] = 0;
    char *slash = strrchr(p, 0x5c);
    if (slash)
        p = slash + 1;
    Resource *res = 0;
    if (strlen(p))
        res = FindResource(p);
    if (res) {
        DebugLog(kResFound, p);
        return res;
    }
    res = FindResource(name);
    if (res) {
        DebugLog(kResFound, name);
        return res;
    }
    char saved[0x80];
    if (strlen(g_dataDir)) {
        GetCurrentDirectory(0x80, saved);
        SetCurrentDirectory(g_dataDir);
    }
    res = LoadResource(name, name);
    if (strlen(g_dataDir))
        SetCurrentDirectory(saved);
    if (res) {
        DebugLog(kResFound, p);
        return res;
    }
    DebugLog(kResMissing, name);
    return 0;
}
