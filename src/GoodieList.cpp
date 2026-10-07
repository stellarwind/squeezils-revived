// GoodieList: see GoodieList.h.
#include <string.h>
#include <stdio.h>
#include <new>
#include "GoodieList.h"
#include "IniReader.h"
#include "Container.h"
#include "MoodFace.h"
#include "Original.h"

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

GoodieList::GoodieList()
{
    count = 0;
    canvas = NULL;
}

GoodieList::~GoodieList()
{
    if (count > 0) {
        for (int i = 0; i < count; i++) {
            FreeBitmap(entries[i].frame0);
            FreeBitmap(entries[i].frame1);
        }
        Operator_Delete(entries);
    }
    FreeBitmap(canvas);
}

BOOL GoodieList::Init(int px, int py)
{
    IniReader ini;
    x = px;
    y = py;
    count = 0;
    ini.SetSection("GoodieList");
    StringList *keys = ini.GetKeys();
    if (keys->count > 0)
        entries = (GoodieEntry *)Operator_New(keys->count * sizeof(GoodieEntry));
    for (int i = 0; i < keys->count; i++) {
        int id = 0;
        sscanf(keys->At(i).c_str(), "%d", &id);
        ini.SetSection("GoodieList");
        char sectionName[20];
        strcpy(sectionName, ini.GetString(keys->At(i).c_str()));
        ini.SetSection(sectionName);
        char picture[20];
        strcpy(picture, ini.GetString("UIProp"));
        GoodieEntry &e = entries[count];
        if (strlen(picture) == 0) {                       // a section without a picture: leave the slot unused
            e.id = -1;
            strcpy(e.section, kEmptyString);
            strcpy(e.picture, kEmptyString);
            e.frame0 = NULL;
            e.frame1 = NULL;
        } else {
            e.id = id;
            strcpy(e.section, sectionName);
            strcpy(e.picture, picture);
            e.frame0 = NewBitmap();
            e.frame1 = NewBitmap();
            count++;
        }
    }
    if (keys)
        delete keys;
    return TRUE;
}

BOOL GoodieList::Load()
{
    char dir[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\\ui\\");
    width = 0;
    height = 0;
    for (int i = 0; i < count; i++) {
        GoodieEntry &e = entries[i];
        if (e.id == -1)
            continue;
        char base[100];
        strcpy(base, dir);
        strcat(base, e.picture);
        char path[100];
        strcpy(path, base);
        strcat(path, ".bmp");
        if (!e.frame0->Load(path))
            return FALSE;
        if (e.frame0->GetWidth() > width)
            width = e.frame0->GetWidth();
        if (e.frame0->GetHeight() > height)
            height = e.frame0->GetHeight();
        strcat(base, "1.bmp");
        e.frame1->Load(base);
    }
    return TRUE;
}

void GoodieList::Draw(int frame, const String &name, Bitmap *panel)
{
    int ox, oy;
    GetScreenOrigin(&ox, &oy);
    LPDIRECTDRAWSURFACE screen = GetRenderer()->primary;
    if (!canvas) {
        canvas = NewBitmap();
        canvas->CreateSurfaceSized(width, height);
        canvas->CaptureFrom(panel->GetSurface(), x, y);
    }
    canvas->DrawToSurfaceAt(screen, x + ox, y + oy, 0, 0, 1);       // put the panel picture back over the last goodie
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
