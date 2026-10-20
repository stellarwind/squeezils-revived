// Body color lookup and INI fallback, SQZ.EXE 0x458f20-0x458fd0.
#include <string.h>
#include "SqueezilBody/BodyColor.h"
#include "IniReader/IniReader.h"
void GameLog(const char *format, ...); // 0x419290

int FindBodyColor(const char *name)
{
    int color;
    for (color = 0; color < 7; ++color) {
        if (_stricmp(name, BodyColorName(color)) == 0)
            break;
    }
    return color == 7 ? 8 : color;
}

int ReadBodyColor(IniReader *ini)
{
    const char *name = ini->GetString("BodyColor");
    if (strlen(name) == 0) {
        name = "RED1";
        GameLog("WARNING: no body color specified, using %s.\n", name);
    }
    int color = FindBodyColor(name);
    if (color == 8) {
        color = 6;
        GameLog("WARNING: unrecognized body color %s specified, using %s.\n",
            name, BodyColorName(color));
    }
    return color;
}
