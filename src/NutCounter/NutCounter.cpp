// NutCounter (SQZ.EXE 0x42f780-0x42fc22), functions in address order.
#include <string.h>
#include "NutCounter/NutCounter.h"
#include "IniReader/IniReader.h"

extern char kGameDirectory[];            // 0x4c73d8
extern char kUiFolder[];                 // "\ui\" 0x4b9480
extern char kInterfaceSection[];         // "Interface" 0x4b9474
extern char kCouldNotCopy[];             // 0x4bc020
extern Renderer *g_renderer;             // 0x4c75dc
Panel *GetPanel();                       // 0x4206c0
void GameLog(const char *format, ...);   // 0x419290

NutCounter::NutCounter()
{
    icon = 0;
    original = 0;
}

NutCounter::~NutCounter()
{
    if (icon)
        delete icon;
    if (original)
        delete original;
}

int NutCounter::Init(const char *key, int px, int py, int m)
{
    IniReader ini;
    ini.SetSection(kInterfaceSection);
    strcpy(file, ini.GetString(key));
    x = px;
    y = py;
    icon = new Bitmap(g_renderer);
    original = new Bitmap(g_renderer);
    mode = m;
    return 1;
}

int NutCounter::Load()
{
    char dir[100];
    char path[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, kUiFolder);
    strcpy(path, dir);
    strcat(path, file);
    if (icon->Load(path)) {
        original->CopyFrom(icon);
        return 1;
    }
    return 0;
}

void NutCounter::Draw(int value, int value2)
{
    lastValue = value;
    if (!icon->CopyFrom(original))
        GameLog(kCouldNotCopy);
    if (mode != 1) {
        DrawNumber(value, icon->GetWidth() / 2, icon->GetHeight() / 2);
    } else {
        DrawNumber(value, icon->GetWidth() / 2, icon->GetHeight() / 2 - icon->GetHeight() / 4);
        DrawNumber(value2, icon->GetWidth() / 2, icon->GetHeight() / 2 + icon->GetHeight() / 4);
    }
    int ox, oy;
    GetPanel()->GetScreenOrigin(&ox, &oy);
    IDirectDrawSurface *s = g_renderer->primary;
    icon->DrawToSurfaceAt(s, x + ox, y + oy, 0, 0, 1);
}

void NutCounter::DrawNumber(int value, int px, int py)
{
    int left, top;
    if (value >= 10) {
        left = px - GetPanel()->digits[value % 10]->GetWidth() / 2 - GetPanel()->digits[value / 10]->GetWidth() / 2;
        top = py - GetPanel()->digits[value / 10]->GetHeight() / 2;
        GetPanel()->digits[value / 10]->DrawToBitmap(icon, left, top, 0, 0);
        left += GetPanel()->digits[value / 10]->GetWidth();
        value = value % 10;
    } else {
        left = px - GetPanel()->digits[value]->GetWidth() / 2;
        top = py - GetPanel()->digits[value]->GetHeight() / 2;
    }
    GetPanel()->digits[value]->DrawToBitmap(icon, left, top, 0, 0);
}
