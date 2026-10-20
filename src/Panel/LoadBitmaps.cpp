// 0x42e500: load the panel background, compass pictures and ten digit sprites.
#include "Panel/Panel.h"
#include "Bitmap/Bitmap.h"
#include "IniReader/IniReader.h"
#include <stdio.h>
#include <string.h>
#pragma intrinsic(strcpy, strcat)
extern char g_panelGameDirectory[]; // 0x4c73d8
void PanelBitmapLog(const char *, ...); // 0x419290
struct PanelCompassLoader { int Load(); }; // 0x414c10
int Panel::LoadBitmaps()
{
    IniReader ini;
    ini.SetSection("Interface");
    strcpy(backgroundFile, ini.GetString("Panel"));
    PanelBitmapLog("reading Panel bitmaps \n");
    char directory[100], path[100];
    strcpy(directory, g_panelGameDirectory);
    strcat(directory, "\\ui\\");
    if (background) {
        strcpy(path, directory);
        strcat(path, backgroundFile);
        if (!background->Load(path))
            return 0;
    }
    if (!((PanelCompassLoader *)pad480)->Load())
        return 0;
    for (int i = 0; i < 10; ++i) {
        sprintf(path, "%snumb_%d.bmp", directory, i);
        if (!((Bitmap *)digits[i])->Load(path))
            return 0;
    }
    PanelBitmapLog("Panel bitmaps Read\n");
    return 1;
}
