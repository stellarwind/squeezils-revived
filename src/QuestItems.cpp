// QuestItems: see QuestItems.h.
#include <string.h>
#include <stdlib.h>
#include <new>
#include "QuestItems.h"
#include "IniReader.h"
#include "Renderer.h"
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
    *(int *)(pad60 + 4) = b;
    *(int *)(pad60 + 0) = a;
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
