// SQZ.EXE 0x43dcd0 (ret 4): which mouse button (1-4) is bound by default to this command, 0 for none.
#include <string.h>
#include "IniReader/IniReader.h"

struct BindEntry {
    char before[0x24];
    char name[0x20];                                     // 0x24
};
struct BindTable {
    BindEntry entries[0x33];
    int MouseButton(int command);
};

int BindTable::MouseButton(int command)
{
    IniReader ini;
    ini.SetSection("MouseDefaults");
    const char *b1 = ini.GetString("Button1");
    char *name = entries[command].name;
    if (!strcmp(b1, name))
        return 1;
    if (!strcmp(ini.GetString("Button2"), name))
        return 2;
    if (!strcmp(ini.GetString("Button3"), name))
        return 3;
    if (!strcmp(ini.GetString("Button4"), name))
        return 4;
    return 0;
}
