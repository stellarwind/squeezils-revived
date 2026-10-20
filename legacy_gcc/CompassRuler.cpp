// CompassRuler: see CompassRuler.h.
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <new>
#include "CompassRuler.h"
#include "MoodFace.h"
#include "IniReader.h"
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

CompassRuler::CompassRuler()
{
    for (int i = 0; i < 10; i++)
        picture[i] = NULL;
    picture[10] = NULL;
    picture[11] = NULL;
    pineRegion = desertRegion = coastRegion = jungleRegion = 0;
}

void CompassRuler::FreeBitmaps()
{
    // the original frees them in this order; the pointers are cleared so that a failed Load followed by the destructor is safe
    static const int order[12] = { 0, 2, 8, 6, 4, 1, 3, 9, 7, 5, 10, 11 };
    for (int i = 0; i < 12; i++) {
        FreeBitmap(picture[order[i]]);
        picture[order[i]] = NULL;
    }
}

CompassRuler::~CompassRuler()
{
    FreeBitmaps();
}

BOOL CompassRuler::Init()
{
    IniReader ini;
    ini.SetSection("Compass");
    // key in [Compass], where the name goes, and which two pictures it is for
    struct { const char *key; int nameIndex; int first; } places[6] = {
        { "River", 0, 0 }, { "Coast", 1, 2 }, { "Desert", 2, 4 }, { "Pine", 5, 6 }, { "Jungle", 3, 8 }, { "Spacer", 4, 10 },
    };
    for (int i = 0; i < 6; i++) {
        strcpy(name[places[i].nameIndex], ini.GetString(places[i].key));
        picture[places[i].first] = NewBitmap();
        picture[places[i].first + 1] = NewBitmap();
    }

    // [RegionList] in the game's ini: "<number> = regions\<place>"; remember each place's number (minus one)
    char keys[100];
    GetPrivateProfileStringA("RegionList", NULL, "", keys, 100, kCurrentIniPath);
    for (int at = 0; keys[at]; ) {
        const char *key = &keys[at];
        char value[20];
        GetPrivateProfileStringA("RegionList", key, "", value, 20, kCurrentIniPath);
        const char *place = value + 8;                   // past "regions\"
        if (strcmp(place, "pine") == 0)
            pineRegion = atoi(key) - 1;
        if (strcmp(place, "jungle") == 0)
            jungleRegion = atoi(key) - 1;
        if (strcmp(place, "desert") == 0)
            desertRegion = atoi(key) - 1;
        if (strcmp(place, "coast") == 0)
            coastRegion = atoi(key) - 1;
        at += (int)strlen(key) + 1;
    }
    UpdateSprites();
    x = 261;
    y = 29;
    return TRUE;
}

BOOL CompassRuler::UpdateSprites()
{
    // picture pairs: coast (2, 3), desert (4, 5), pine (6, 7), jungle (8, 9), spacer (10)
    Bitmap *spacer = picture[10];
    if (!GetCurrentSimulation()) {
        sprite[0] = picture[6];
        sprite[1] = spacer;
        sprite[2] = picture[4];
        sprite[3] = spacer;
        sprite[4] = picture[2];
        sprite[5] = spacer;
        sprite[6] = picture[8];
        sprite[7] = spacer;
    } else {
        int *visited = (int *)PlayerInventory();                 // one flag per region, from byte 0x58 of the table
        if (!GetPlayerSoul() || !visited)
            return TRUE;
        visited += 0x58 / 4;
        sprite[0] = visited[pineRegion] == 0 ? picture[6] : picture[7];
        sprite[1] = spacer;
        sprite[2] = visited[desertRegion] == 0 ? picture[4] : picture[5];
        sprite[3] = spacer;
        sprite[4] = visited[coastRegion] == 0 ? picture[2] : picture[3];
        sprite[5] = spacer;
        sprite[6] = visited[jungleRegion] == 0 ? picture[8] : picture[9];
        sprite[7] = spacer;
    }
    sector = 3;
    sectorCount = 8;
    fullTurn = 6.283185f;
    quarter = 0.785398f;
    eighth = 0.392699f;
    return TRUE;
}

BOOL CompassRuler::LoadPair(Bitmap **pair, const char *file)
{
    strcpy(path, kGameDirectory);
    strcat(path, "\\ui\\");
    char full[100];
    strcpy(full, path);
    strcat(full, file);
    strcat(full, ".bmp");
    if (!pair[0]->Load(full)) {
        FreeBitmaps();
        return FALSE;
    }
    strcpy(full, path);
    strcat(full, file);
    strcat(full, "1");
    strcat(full, ".bmp");
    if (!pair[1]->Load(full)) {
        FreeBitmaps();
        return FALSE;
    }
    return TRUE;
}

BOOL CompassRuler::Load()
{
    if (!LoadPair(&picture[2], name[1])) return FALSE;          // coast
    if (!LoadPair(&picture[4], name[2])) return FALSE;          // desert
    char full[100];
    strcpy(full, path);                                         // the folder left in `path` by the last LoadPair
    strcat(full, name[4]);                                      // the spacer has only the one picture
    strcat(full, ".bmp");
    if (!picture[10]->Load(full)) {
        FreeBitmaps();
        return FALSE;
    }
    if (!LoadPair(&picture[8], name[3])) return FALSE;          // jungle
    if (!LoadPair(&picture[6], name[5])) return FALSE;          // pine
    if (!picture[11]->CopyFrom(picture[6])) {                   // the spacer's second picture is a copy of the pine's first
        FreeBitmaps();
        return FALSE;
    }
    return TRUE;
}

void CompassRuler::SetCompass(float angle, float roll)
{
    LPDIRECTDRAWSURFACE screen = GetRenderer()->primary;
    if (!screen)
        return;
    double whole;
    double fraction = modf((double)angle / (double)quarter, &whole);
    int s = (int)whole;
    sector = s;
    if (sector >= sectorCount)
        sector = 0;
    if (sector < 0)
        sector = sectorCount - 1;
    s = sector;
    int other = s + 1;
    if (!((double)(s + 1) * (double)eighth < (double)angle)) {
        other = s - 1;
        if (!((double)quarter > (double)angle))
            other = s;
    }
    if (other >= sectorCount)
        other = 0;
    if (other < 0)
        other = sectorCount - 1;
    Compose(s, other, (float)fabs(fraction), roll);
    int ox, oy;
    GetScreenOrigin(&ox, &oy);
    picture[11]->DrawToSurfaceAt(screen, x + ox, y + oy, 0, 0, 1);
}

static unsigned BitsOf(float f)
{
    unsigned u;
    memcpy(&u, &f, sizeof u);
    return u;
}

BOOL CompassRuler::Compose(int a, int b, float fraction, float roll)
{
    Bitmap *strip = picture[11];
    Bitmap *from = sprite[a];
    Bitmap *next = sprite[b];
    int w = from->GetWidth();
    int left = (int)((1.0 - (double)fraction) * (double)w);          // how much of the strip comes from this sector's picture
    int right = w - left;                                            // and how much from the neighbour's
    float rollSize = fabsf(roll);
    int h = from->GetHeight();
    int shift = (int)((double)h * (double)rollSize);                 // rows the picture is scrolled up or down
    if (BitsOf(roll) > 0x80000000u)
        shift = h - shift;

    if (left > 0 && !strip->CopyRectFrom(from, right, shift, w, h, 0, 0, left, h - shift)) {
        GameLog("could not copy surface\n");
        return FALSE;
    }
    if (right <= 0)
        return TRUE;
    int shiftNext = (int)((double)from->GetHeight() * (double)rollSize);
    if (BitsOf(roll) > 0x80000000u)
        shiftNext = from->GetHeight() - shiftNext;
    int hFrom = from->GetHeight();
    if (!strip->CopyRectFrom(next, 0, shiftNext, right, hFrom, left, 0, w, hFrom - shiftNext)) {
        GameLog("could not copy surface\n");
        return FALSE;
    }
    if ((BitsOf(roll) & 0x7fffffff) == 0)
        return TRUE;
    // the rows scrolled off the top come back round at the bottom
    if (left > 0 && !strip->CopyRectFrom(from, right, 0, w, shift, 0, h - shift, left, from->GetHeight())) {
        GameLog("could not copy surface\n");
        return FALSE;
    }
    if (right <= 0)
        return TRUE;
    if (!strip->CopyRectFrom(next, 0, 0, right, shiftNext, left, hFrom - shiftNext, w, next->GetHeight())) {
        GameLog("could not copy surface\n");
        return FALSE;
    }
    return TRUE;
}
