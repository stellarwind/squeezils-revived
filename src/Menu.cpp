// Menu: see Menu.h.
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <new>
#include "Menu.h"
#include "IniReader.h"
#include "Renderer.h"
#include "Original.h"

static void FreeBitmap(Bitmap *b)
{
    b->~Bitmap();
    Operator_Delete(b);
}

Menu::Menu()
{
    vtbl = (void **)kMenuVtable;
    IniReader ini;
    ini.SetSection("Interface");
    itemCount = atoi(ini.GetString("MenuItems"));
    for (int i = 0; i < MAX_ITEMS; i++) {
        items[i].normal = NULL;
        items[i].lit = NULL;
        items[i].action = NULL;
        items[i].highlighted = 0;
    }
    GetModuleFileNameA(NULL, kSaveGameDirectory, 100);
    char *slash = strrchr(kSaveGameDirectory, '\\');
    if (slash) *slash = 0;
    strcat(kSaveGameDirectory, "\\sqz\\save\\");
}

Menu::~Menu()
{
    vtbl = (void **)kMenuVtable;
    for (int i = 0; i < itemCount; i++) {
        Bitmap *b = items[i].normal;
        if (b && i < MAX_ITEMS) {
            FreeBitmap(b);
            if (items[i].lit)
                FreeBitmap(items[i].lit);
        }
    }
}

BOOL Menu::LoadItemBitmap(const char *file, Bitmap *bmp)
{
    char dir[100], path[100];
    strcpy(dir, kGameDirectory);
    strcat(dir, "\\ui\\");
    strcpy(path, dir);
    strcat(path, file);
    return bmp->Load(path) != 0;
}

BOOL Menu::Load(int a1, int a2)
{
    IniReader ini;
    ini.SetSection("Interface");
    for (int i = 0; i < itemCount; i++) {
        char key[40], file[100];
        sprintf(key, "Menu_%d", i + 1);
        const char *base = ini.GetString(key);
        if (i < MAX_ITEMS) {
            void *m = Operator_New(sizeof(Bitmap));
            items[i].normal = m ? new (m) Bitmap(GetRenderer()) : NULL;
            strcpy(file, base);
            strcat(file, ".bmp");
            if (!LoadItemBitmap(file, items[i].normal))
                return FALSE;
            m = Operator_New(sizeof(Bitmap));
            items[i].lit = m ? new (m) Bitmap(GetRenderer()) : NULL;
            strcpy(file, base);
            strcat(file, "2.bmp");
            if (!LoadItemBitmap(file, items[i].lit))
                return FALSE;
        }
        char keyName[100];
        strcpy(keyName, "");
        strcat(keyName, key);
        strcat(keyName, "Key");
        if (i < MAX_ITEMS)                                // the original also writes past the tenth record here
            items[i].key = GetKeyCode(ini.GetString(keyName));
    }
    SetActions();
    arg1 = a1;
    arg2 = a2;
    left = 0;
    int maxWidth = 0, maxHeight = 0, totalHeight = 0;
    spacing = 0;
    for (int i = 0; i < itemCount; i++) {
        Bitmap *b = items[i].normal;
        if (b->GetWidth() > maxWidth) maxWidth = b->GetWidth();
        if (b->GetHeight() > maxHeight) maxHeight = b->GetHeight();
        totalHeight += b->GetHeight();
    }
    ScreenInfo *screen = GetScreenInfo();
    left = screen->width / 2 - maxWidth / 2;
    itemHeight = maxHeight;
    top = screen->height / 2 - (totalHeight + spacing * itemCount) / 2;
    return TRUE;
}

void Menu::SetActions()
{
    for (int i = 0; i < itemCount; i++) {
        switch (i) {
        case 0: items[i].action = ActionStart; break;
        case 1: items[i].action = ActionLoadGame; break;
        case 2: items[i].action = ActionOptions; break;
        case 3: items[i].action = ActionHelp; break;
        case 4: items[i].action = ActionCredits; break;
        case 5: items[i].action = ActionQuit; break;
        }
    }
}

void Menu::SaveAnchor()
{
    anchorX = left;
    anchorY = top;
}

void Menu::DrawItem(int index, int lit)
{
    Renderer *r = GetRenderer();
    if (r->field_078) return;
    SaveAnchor();
    LPDIRECTDRAWSURFACE primary = GetRenderer()->primary;
    Item &it = items[index];
    POINT pt = { it.left, it.top };
    ClientToScreen(GetRenderer()->hwnd, &pt);
    Bitmap *b = lit ? it.lit : it.normal;
    b->DrawToSurfaceAt(primary, pt.x, pt.y, 0, 0, 1);
    it.highlighted = lit;
}

void Menu::Relayout()
{
    SaveAnchor();
    for (int i = 0; i < itemCount; i++) {
        int x = anchorX;
        int y = (spacing + itemHeight) * i + anchorY;
        if (i < MAX_ITEMS) {
            items[i].left = x;
            items[i].right = x + items[i].normal->GetWidth();
            items[i].top = y;
            items[i].bottom = y + items[i].normal->GetHeight();
            DrawItem(i, 0);
        }
    }
    SetSelected(0);
}

int Menu::SetSelected(int index)
{
    for (int i = 0; i < itemCount; i++) {
        if (!items[i].highlighted) {
            if (i == index) DrawItem(i, 1);
        } else if (i != index) {
            DrawItem(i, 0);
        }
    }
    return 0;
}

int Menu::HitTest(int index, int x, int y)
{
    const Item &it = items[index];
    return it.left <= x && x <= it.right && it.top < y && y <= it.bottom;
}

int Menu::OnMouseMove(int x, int y)
{
    int moved = 0;
    for (int i = 0; i < itemCount; i++) {
        if (!items[i].highlighted && HitTest(i, x, y)) {
            DrawItem(i, 1);
            moved = 1;
        }
    }
    if (!moved) return 0;
    for (int i = 0; i < itemCount; i++) {
        if (!HitTest(i, x, y))
            DrawItem(i, 0);
    }
    return 0;
}

LRESULT Menu::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam, WPARAM *lastKey)
{
    int result = 0;
    if (msg == WM_SYSKEYDOWN || msg == WM_SYSCOMMAND) {
        if ((wParam & 0xfff0) == SC_CLOSE || wParam == VK_F4)
            return ActionQuit();
        return 0;
    }
    if (msg == WM_KEYDOWN) {
        int key = (int)wParam;
        *lastKey = wParam;
        for (int i = 0; i < itemCount; i++) {
            if (items[i].key == key) {
                if (items[i].action) result = items[i].action();
                break;
            }
        }
        int step = 1, selected = -1;
        if (key == VK_UP) {
            step = -1;
        } else if (key != VK_DOWN) {
            if (key == VK_RETURN) {
                for (int i = 0; i < itemCount; i++)
                    if (items[i].highlighted)
                        result = items[i].action();
            }
            return result;
        }
        for (int i = 0; i < itemCount; i++) {
            if (items[i].highlighted) {
                selected = i + step;
                if (selected >= itemCount) selected = 0;
                else if (selected < 0) selected = itemCount - 1;
            }
        }
        if (selected == -1) selected = 0;
        return SetSelected(selected);
    }
    if (msg == WM_MOUSEMOVE)
        return OnMouseMove(LOWORD(lParam), HIWORD(lParam));
    if (msg == WM_LBUTTONDOWN) {
        int x = LOWORD(lParam), y = HIWORD(lParam);
        for (int i = 0; i < itemCount; i++) {
            if (HitTest(i, x, y) && items[i].highlighted && items[i].action)
                result = items[i].action();
        }
    }
    return result;
}

int __cdecl Menu::ActionStart()
{
    return 2;
}

int __cdecl Menu::ActionLoadGame()
{
    char path[200];
    strcpy(kLoadGameName, "");
    strcpy(kLoadGameTitle, "");
    if (DialogBoxParamA(GetInstance(), MAKEINTRESOURCE(113), GetMainWindow(), (DLGPROC)kLoadGameDialogProc, 0)) {
        if (strlen(kLoadGameName) != 0) {
            strcpy(path, kSaveGameDirectory);
            strcat(path, kLoadGameName);
            if (!GameLoadGame(path)) {
                strcpy(path, "Could not load game: ");
                strcat(path, kLoadGameName);
                MessageBoxA(GetRenderer()->hwnd, path, "Load Game", 0);
            }
        }
    }
    return 0;
}

int __cdecl Menu::ActionOptions()
{
    DialogBoxParamA(GetInstance(), MAKEINTRESOURCE(129), GetMainWindow(), (DLGPROC)kAboutDialogProc, 0);
    return 0;
}

int __cdecl Menu::ActionHelp()
{
    WinHelpA(GetRenderer()->hwnd, ".\\help\\sqz.hlp", HELP_CONTEXT, 1);
    return 0;
}

int __cdecl Menu::ActionCredits()
{
    WinHelpA(GetRenderer()->hwnd, ".\\help\\sqz.hlp", HELP_CONTEXT, 2);
    return 0;
}

int __cdecl Menu::ActionQuit()
{
    return DialogBoxParamA(GetInstance(), MAKEINTRESOURCE(128), GetMainWindow(), (DLGPROC)kQuitDialogProc, 0) != 0;
}
