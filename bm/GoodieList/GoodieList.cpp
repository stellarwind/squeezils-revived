// GoodieList (SQZ.EXE 0x4156c0-0x415ee0), functions in address order.
#include <string.h>
#include <stdio.h>
#include "GoodieList/GoodieList.h"
#include "IniReader/IniReader.h"

extern char kGameDirectory[];            // 0x4c73d8
extern char kUiFolder[];                 // "\ui\" 0x4b9480
extern char kBmpExt[];                   // ".bmp"
extern char kBmp1Ext[];                  // "1.bmp"
extern Renderer *g_renderer;             // 0x4c75dc
class Panel { public: void GetScreenOrigin(int *x, int *y); };   // 0x42da30
Panel *GetPanel();                                                // 0x4206c0

GoodieList::GoodieList()
{
    count = 0;
    canvas = 0;
}

GoodieList::~GoodieList()
{
    int i;
    if (count > 0) {
        for (i = 0; i < count; i++) {
            if (entries[i].frame0)
                delete entries[i].frame0;
            if (entries[i].frame1)
                delete entries[i].frame1;
        }
        operator delete(entries);
    }
    if (canvas)
        delete canvas;
}

int GoodieList::Init(int px, int py)
{
    IniReader ini;
    x = px;
    y = py;
    count = 0;
    ini.SetSection("GoodieList");
    StringList *keys = ini.GetKeys();
    if (keys->count > 0)
        entries = (GoodieEntry *)operator new(keys->count * sizeof(GoodieEntry));
    for (int i = 0; i < keys->count; i++) {
        int id = 0;
        sscanf(keys->Get(i).c_str(), "%d", &id);
        ini.SetSection("GoodieList");
        char sectionName[20];
        strcpy(sectionName, ini.GetString(keys->Get(i).c_str()));
        ini.SetSection(sectionName);
        char picture[20];
        strcpy(picture, ini.GetString("UIProp"));
        if (strlen(picture) == 0) {
            entries[count].id = -1;
            strcpy(entries[count].section, kEmptyString);
            strcpy(entries[count].picture, kEmptyString);
            entries[count].frame0 = 0;
            entries[count].frame1 = 0;
        } else {
            entries[count].id = id;
            strcpy(entries[count].section, sectionName);
            strcpy(entries[count].picture, picture);
            entries[count].frame0 = new Bitmap(g_renderer);
            entries[count].frame1 = new Bitmap(g_renderer);
            count++;
        }
    }
    if (keys)
        delete keys;
    return 1;
}

int GoodieList::Load()
{
    char dir[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, kUiFolder);
    width = 0;
    height = 0;
    for (int i = 0; i < count; i++) {
        if (entries[i].id == -1)
            continue;
        char base[100];
        char path[100];
        strcpy(base, dir);
        strcat(base, entries[i].picture);
        strcpy(path, base);
        strcat(base, ".bmp");
        if (!entries[i].frame0->Load(base))
            return 0;
        if (entries[i].frame0->GetWidth() > width)
            width = entries[i].frame0->GetWidth();
        if (entries[i].frame0->GetHeight() > height)
            height = entries[i].frame0->GetHeight();
        strcat(path, "1.bmp");
        entries[i].frame1->Load(path);
    }
    return 1;
}

void GoodieList::Draw(int frame, String name, Bitmap *panel)
{
    int ox, oy;
    GetPanel()->GetScreenOrigin(&ox, &oy);
    IDirectDrawSurface *screen = g_renderer->primary;
    if (!canvas) {
        canvas = new Bitmap(g_renderer);
        canvas->CreateSurfaceSized(width, height);
        canvas->CaptureFrom(panel->GetSurface(), x, y);
    }
    canvas->DrawToSurfaceAt(screen, x + ox, y + oy, 0, 0, 0);
    int i = 0;
    for (; i < count; i++) {
        if (strcmp(name.c_str(), entries[i].section) == 0)
            break;
    }
    if (count <= i)
        return;
    Bitmap *picture = frame == 0 ? entries[i].frame0 : entries[i].frame1;
    if (!picture)
        return;
    picture->DrawToSurfaceAt(screen, x + ox, y + oy, 0, 0, 1);
}
