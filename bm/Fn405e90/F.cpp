// Menu::Menu (0x405e90): reads the item count from the [Interface] section, clears the ten item slots and works out the save directory.
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include "IniReader/IniReader.h"

extern char g_saveDir[];       // 0x4c6400
struct MenuItem { int a, b; char pad[0x10]; int c; int pad2; int d; };   // 0x24 bytes
struct MenuC {
    virtual int SetSelected(int index);
    int itemCount;             // 0x04
    char pad[0x18 - 8];
    MenuItem items[10];        // 0x18
    MenuC();
};
MenuC::MenuC()
{
    int i = 0;
    IniReader ini;
    ini.SetSection("Interface");
    itemCount = atoi(ini.GetString("MenuItems"));
    for (; i < 10; i++) {
        items[i].a = 0;
        items[i].b = 0;
        items[i].c = 0;
        items[i].d = 0;
    }
    GetModuleFileName(0, g_saveDir, 100);
    char *p = strrchr(g_saveDir, '\\');
    if (p)
        *p = 0;
    strcat(g_saveDir, "\\sqz\\save\\");
}
