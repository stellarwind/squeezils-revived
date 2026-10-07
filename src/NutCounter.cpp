// NutCounter: see NutCounter.h.
#include <string.h>
#include <new>
#include "NutCounter.h"
#include "MoodFace.h"
#include "IniReader.h"
#include "Renderer.h"
#include "Original.h"

// the panel's ten digit pictures, 0 to 9
static Bitmap **Digits()
{
    return (Bitmap **)((char *)GetPanel() + 0x428);
}

NutCounter::NutCounter()
{
    icon = NULL;
    original = NULL;
}

NutCounter::~NutCounter()
{
    if (icon) {
        icon->~Bitmap();
        Operator_Delete(icon);
    }
    if (original) {
        original->~Bitmap();
        Operator_Delete(original);
    }
}

static Bitmap *NewBitmap()
{
    void *m = Operator_New(sizeof(Bitmap));
    return m ? new (m) Bitmap(GetRenderer()) : NULL;
}

BOOL NutCounter::Init(const char *key, int px, int py, int m)
{
    IniReader ini;
    ini.SetSection("Interface");
    strcpy(file, ini.GetString(key));
    x = px;
    y = py;
    icon = NewBitmap();
    original = NewBitmap();
    mode = m;
    return TRUE;
}

BOOL NutCounter::Load()
{
    char dir[100], path[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\\ui\\");
    strcpy(path, dir);
    strcat(path, file);
    if (!icon->Load(path))
        return FALSE;
    original->CopyFrom(icon);
    return TRUE;
}

void NutCounter::DrawNumber(int value, int px, int py)
{
    Bitmap **d = Digits();
    if (value >= 10) {
        int tens = value / 10, ones = value % 10;       // the original indexes past the digits for 100 and over
        if (tens > 9) tens = 9;
        int left = px - d[ones]->GetWidth() / 2 - d[tens]->GetWidth() / 2;
        int top = py - d[tens]->GetHeight() / 2;
        d[tens]->DrawToBitmap(icon, left, top, 0, 0);
        left += d[tens]->GetWidth();
        d[ones]->DrawToBitmap(icon, left, top, 0, 0);
    } else {
        int left = px - d[value]->GetWidth() / 2;
        int top = py - d[value]->GetHeight() / 2;
        d[value]->DrawToBitmap(icon, left, top, 0, 0);
    }
}

void NutCounter::Draw(int value, int value2)
{
    lastValue = value;
    if (!icon->CopyFrom(original))
        GameError("could not copy");
    int w = icon->GetWidth(), h = icon->GetHeight();
    if (mode == 1) {
        DrawNumber(value, w / 2, h / 2 - h / 4);
        DrawNumber(value2, w / 2, h / 2 + h / 4);
    } else {
        DrawNumber(value, w / 2, h / 2);
    }
    int ox, oy;
    GetScreenOrigin(&ox, &oy);
    icon->DrawToSurfaceAt(GetRenderer()->primary, x + ox, y + oy, 0, 0, 1);
}
