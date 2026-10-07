// Panel: see Panel.h.
#include <string.h>
#include <stdio.h>
#include <new>
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

static Bitmap *NewBitmap()
{
    void *m = Operator_New(sizeof(Bitmap));
    return m ? new (m) Bitmap(GetRenderer()) : NULL;
}

static void FreeBitmap(Bitmap *b)
{
    if (b) {
        b->~Bitmap();
        Operator_Delete(b);
    }
}

static float FloatFromBits(unsigned bits)
{
    float f;
    memcpy(&f, &bits, sizeof f);
    return f;
}

Panel::Panel()
{
    // status and quest are built first (their constructors run before this body), then the region screen
    RegionScreenCtor(regionScreen);
    background = NewBitmap();
    field478 = 0;
    field47c = 0;
    RegionScreenInit(regionScreen);
    for (int i = 0; i < 10; i++) {
        digits[i] = 0;
        digits[i] = NewBitmap();
    }
    GetScreenOrigin(&originX, &originY);
    vec604.x = vec604.y = vec604.z = -5.0f;
    field610 = FloatFromBits(0x40c90fd0);
    field5bc = 0.01f;
    IniReader ini;
    ini.SetSection("Compass");
    verticalChange = ini.GetFloat("VerticalChange", 1.0f);
    verticalThreshold = ini.GetFloat("VerticalThreshold", 0.1f);
    field450 = 2;
    field454 = 2;
    field45c = 0;
    field614 = 0;
    field458 = -1;
    font = NULL;
    PanelLoadFonts(this);
    messageFont[0] = 0;
    pad694[0] = 0;
}

Panel::~Panel()
{
    FreeBitmap(background);
    for (int i = 0; i < 10; i++)
        FreeBitmap(digits[i]);
    if (font)
        DeleteObject(font);
    RegionScreenDtor(regionScreen);        // then quest and status are destroyed, in that order
}
