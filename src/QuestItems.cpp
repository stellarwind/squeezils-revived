// QuestItems: see QuestItems.h.
#include <string.h>
#include <stdlib.h>
#include <new>
#include "QuestItems.h"
#include "IniReader.h"
#include "Renderer.h"
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

QuestItems::QuestItems()
{
    combined = NULL;
    left = NULL;
    middle = NULL;
    right = NULL;
}

QuestItems::~QuestItems()
{
    FreeBitmap(combined);
    FreeBitmap(left);
    FreeBitmap(middle);
    FreeBitmap(right);
}

BOOL QuestItems::LoadCombined(int)
{
    char dir[100], path[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\\ui\\");
    strcpy(path, dir);
    strcat(path, combinedFile);
    combined->Load(path);
    return FALSE;
}

BOOL QuestItems::Init(int a, int b, int index)
{
    char iniPath[100];
    strcpy(iniPath, GetPlaceName(index));
    strcat(iniPath, "\\region\\region.ini");
    IniReader ini(iniPath);
    ini.SetSection("UIResources");
    strcpy(combinedFile, ini.GetString("CombinedKey"));
    char num[20];
    strcat(combinedFile, _itoa(index, num, 10));
    strcat(combinedFile, ".bmp");
    combined = NewBitmap();
    y = b;
    x = a;
    if (LoadCombined(index))
        return TRUE;
    LoadKeys();
    return FALSE;
}

BOOL QuestItems::LoadKeys()
{
    if (*kQuestItemsLoaded)
        return TRUE;
    char iniPath[100];
    strcpy(iniPath, GetPlaceName(1));
    strcat(iniPath, "\\region\\region.ini");
    IniReader ini(iniPath);
    ini.SetSection("UIResources");
    char dir[100], path[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\\ui\\");

    strcpy(path, dir);
    strcpy(leftFile, ini.GetString("LeftKey"));
    strcat(leftFile, ".bmp");
    strcat(path, leftFile);
    left = NewBitmap();
    if (!left->Load(path)) return FALSE;

    strcpy(path, dir);
    strcpy(middleFile, ini.GetString("MiddleKey"));
    strcat(middleFile, ".bmp");
    strcat(path, middleFile);
    middle = NewBitmap();
    if (!middle->Load(path)) return FALSE;

    strcpy(path, dir);
    strcpy(rightFile, ini.GetString("RightKey"));
    strcat(rightFile, ".bmp");
    strcat(path, rightFile);
    right = NewBitmap();
    if (!right->Load(path)) return FALSE;

    *kQuestItemsLoaded = 1;
    return TRUE;
}

void QuestItems::FreeCombined()
{
    if (combined) {
        combined->~Bitmap();
        Operator_Delete(combined);
    }
}

void QuestItems::Draw(int place)
{
    int ox, oy;
    GetScreenOrigin(&ox, &oy);
    LPDIRECTDRAWSURFACE screen = GetRenderer()->primary;
    combined->DrawToSurfaceAt(screen, x + ox, y + oy, 0, 0, 1);
    int *found = (int *)PlayerInventory();                      // three flags per place, from the second entry of the table
    if (!found)
        return;
    Bitmap *piece[3] = { left, middle, right };
    for (int i = 0; i < 3; i++) {
        if (found[1 + place * 3 + i] == 1)
            piece[i]->DrawToSurfaceAt(screen, x + ox, y + oy, 0, 0, 1);
    }
}
