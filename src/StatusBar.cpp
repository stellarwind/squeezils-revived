// StatusBar: see StatusBar.h.
#include <string.h>
#include <new>
#include "StatusBar.h"
#include "IniReader.h"
#include "Renderer.h"
#include "Original.h"

static Bitmap *NewBitmap()
{
    void *m = Operator_New(sizeof(Bitmap));
    return m ? new (m) Bitmap(GetRenderer()) : NULL;
}

BOOL StatusBar::Init()
{
    amo.Init("AmoNut", 25, 20, 0);
    amoA = 0;
    amoB = -1;
    food.Init("FoodNut", 510, 10, 1);
    foodA = 0;
    foodC = -1;
    if (!GoodieListInit(goodies, 410, 20))
        return FALSE;
    if (!face.Load(150, 14))
        return FALSE;
    deadMessage = NewBitmap();
    messageX = 405;
    messageY = 12;
    wonMessage = NewBitmap();
    return LoadAll() != 0;
}

BOOL StatusBar::LoadAll()
{
    amo.Load();
    food.Load();
    GoodieListLoad(goodies);
    IniReader ini;
    ini.SetSection("Interface");
    char dir[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\\ui\\");
    char name[100], path[100];
    strcpy(name, ini.GetString("DeadMessage"));
    if (deadMessage) {
        strcpy(path, dir);
        strcat(path, name);
        strcat(path, ".bmp");
        if (!deadMessage->Load(path))
            return FALSE;
    }
    strcpy(name, ini.GetString("WonMessage"));
    if (wonMessage) {
        strcpy(path, dir);
        strcat(path, name);
        strcat(path, ".bmp");
        if (!wonMessage->Load(path))
            return FALSE;
    }
    return TRUE;
}
