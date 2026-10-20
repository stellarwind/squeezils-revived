// Menu::Load (SQZ.EXE 0x405ff0, ret 8): for every item read [Interface] Item<n> (the picture base name) and
// Item<n>Action, load the normal and lit pictures (first ten items only), then centre the menu on the screen.
// 0 when a picture fails to load.
#include <string.h>
#include <stdlib.h>
#pragma intrinsic(strcpy, strcat, strlen)
#include "IniReader/IniReader.h"

struct MenuBmp {
    MenuBmp(void *renderer);                  // 0x473760
    int GetWidth();                           // 0x4739b0
    int GetHeight();                          // 0x4739c0
    char d[0x80];
};
struct MenuLoadItem {
    MenuBmp *normal, *lit;                    // 0x00
    int left, top, right, bottom;             // 0x08
    int f18;                                  // 0x18
    int action;                               // 0x1c
    int highlighted;                          // 0x20
};
struct MenuPrefs {
    int ActionId(const char *name);           // 0x43ace0
    int CenterX();                            // 0x43aa30
    int CenterY();                            // 0x43aa40
};
MenuPrefs *GetPrefs();                        // 0x4206b0
extern void *g_app;                           // 0x4c75dc
extern const char kInterface[];               // 0x4b9474
extern const char kNoText[];                  // 0x4b9488
extern const char kItem[], kNormalExt[], kLitExt[], kActionSuffix[];   // 0x4b9550 0x4b943c 0x4b9548 0x4b9544

struct MenuLoad {
    void *vtbl;
    int itemCount;                            // 0x004
    int left, top, itemHeight, spacing;       // 0x008
    MenuLoadItem items[10];                   // 0x018
    int anchorX, anchorY;                     // 0x180
    int arg2, arg1;                           // 0x188
    int LoadItemBitmap(const char *file, MenuBmp *bmp);   // 0x406aa0
    void Relayout();                          // 0x406600
    int Load(int a1, int a2);
};

int MenuLoad::Load(int a1, int a2)
{
    IniReader ini;
    ini.SetSection(kInterface);
    int i = 0;
    if (itemCount > 0) {
        MenuLoadItem *item = items;
        do {
            char file[60];
            char key[16];
            char number[8];
            strcpy(key, kNoText);
            strcpy(key, kItem);
            int next = i + 1;
            itoa(next, number, 10);
            strcat(key, number);
            const char *name = ini.GetString(key);
            strcpy(file, name);
            strcat(file, kNormalExt);
            if (i < 10) {
                item->normal = new MenuBmp(g_app);
                if (!LoadItemBitmap(file, item->normal))
                    return 0;
            }
            strcpy(file, name);
            strcat(file, kLitExt);
            if (i < 10) {
                item->lit = new MenuBmp(g_app);
                if (!LoadItemBitmap(file, item->lit))
                    return 0;
            }
            strcpy(file, kNoText);
            strcat(file, key);
            strcat(file, kActionSuffix);
            item->action = GetPrefs()->ActionId(ini.GetString(file));
            item++;
            i = next;
        } while (itemCount > i);
    }
    Relayout();
    arg1 = a1;
    arg2 = a2;
    left = 0;
    int width = 0;
    int height = 0;
    top = 0;
    int total = 0;
    spacing = 0;
    MenuLoadItem *item = items;
    for (i = 0; i < itemCount; i++, item++) {
        if (item->normal->GetWidth() > width)
            width = item->normal->GetWidth();
        if (item->normal->GetHeight() > height)
            height = item->normal->GetHeight();
        total += item->normal->GetHeight();
    }
    left = GetPrefs()->CenterX() - width / 2;
    itemHeight = height;
    top = GetPrefs()->CenterY() - (total + spacing * itemCount) / 2;
    return 1;
}
