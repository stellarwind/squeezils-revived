// Panel: see Panel.h.
#include <string.h>
#include <stdio.h>
#include "Panel.h"
#include "IniReader.h"
#include "Original.h"

BOOL Panel::LoadBitmaps()
{
    IniReader ini;
    ini.SetSection("Interface");
    strcpy(backgroundFile, ini.GetString("Panel"));
    GameLog("reading Panel bitmaps \n");
    char dir[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\\ui\\");
    if (background) {
        char path[100];
        strcpy(path, dir);
        strcat(path, backgroundFile);
        if (!background->Load(path))
            return FALSE;
    }
    if (!RegionScreenLoad(regionScreen))
        return FALSE;
    for (int i = 0; i < 10; i++) {
        char path[100];
        sprintf(path, "%snumb_%d.bmp", dir, i);
        if (!digits[i]->Load(path))
            return FALSE;
    }
    GameLog("Panel bitmaps Read\n");
    return TRUE;
}
