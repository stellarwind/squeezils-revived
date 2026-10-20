// Menu: the game's main menu, a column of up to ten pictures that highlight under the pointer or the arrow keys and run an
// action when clicked or chosen with Enter. Reconstructed from SQZ.EXE 0x405e90-0x406c20 (RTTI class Menu, vtable 0x49d28c,
// allocated by the application at 0x43a840, 0x190 bytes). The items come from the [Interface] section: MenuItems is the
// count, Menu_<n> names the picture (<name>.bmp normal, <name>2.bmp highlighted) and Menu_<n>Key the key that chooses it.
// the six actions are Start, Load Game, Options, Help, Credits and the quit question. Reading the keyboard (the key
// name -> code table) and the window itself stay in original code.
#pragma once
#include <windows.h>
#include "Bitmap.h"

class Menu {
public:
    enum { MAX_ITEMS = 10 };
    typedef int (__cdecl *Action)();

    struct Item {                 // 0x24 bytes
        Bitmap *normal;           // 0x00 Menu_<n>.bmp
        Bitmap *lit;              // 0x04 Menu_<n>2.bmp, drawn while the item is selected
        int     left, top;        // 0x08 where the picture sits in the window
        int     right, bottom;    // 0x10
        Action  action;           // 0x18 what choosing the item does
        int     key;              // 0x1c virtual key that chooses it
        int     highlighted;      // 0x20 currently drawn in its lit picture
    };

    Menu();                                       // @405e90
    ~Menu();                                      // @405f90
    BOOL Load(int arg1, int arg2);                // @405ff0 load the pictures and keys, lay the items out centred (args kept at +0x18c / +0x188)
    void SetActions();                            // @406600 item n gets action n (0..5)
    BOOL LoadItemBitmap(const char *file, Bitmap *bmp);    // @406aa0 load a picture from the game's ui folder
    void SaveAnchor();                            // @406b80 remember where the column starts
    void Relayout();                              // @406ba0 place every item from the saved anchor, redraw, select the first
    void DrawItem(int index, int lit);            // @406c20 draw one item's picture on the primary surface
    int  SetSelected(int index);                  // @406a10 (slot 0) light the item at index, unlight the others
    int  OnMouseMove(int x, int y);               // @406960 (slot 1) light the item under the pointer
    int  HitTest(int index, int x, int y);        // @406a60 is the point inside the item's picture
    LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam, WPARAM *lastKey);   // @406420 key, mouse and close handling

    // the actions (their dialogs are in Dialogs.cpp)
    static int __cdecl ActionStart();             // @406950 returns 2
    static int __cdecl ActionLoadGame();          // @406760 pick a saved game and load it
    static int __cdecl ActionOptions();          // @406730 the options dialog (and its sub dialogs)
    static int __cdecl ActionHelp();             // @406670 WinHelp topic 1
    static int __cdecl ActionCredits();          // @406690 WinHelp topic 2 (the credits)
    static int __cdecl ActionQuit();              // @406920 the quit question; 1 when answered yes

    void **vtbl;                  // 0x000 0x49d28c
    int    itemCount;             // 0x004 MenuItems (at most ten are loaded)
    int    left;                  // 0x008 top left corner of the column
    int    top;                   // 0x00c
    int    itemHeight;            // 0x010 tallest picture
    int    spacing;               // 0x014 gap between items (0)
    Item   items[MAX_ITEMS];      // 0x018
    int    anchorX;               // 0x180 copy of left/top taken by SaveAnchor
    int    anchorY;               // 0x184
    int    arg2;                  // 0x188 the second argument of Load
    int    arg1;                  // 0x18c the first
};
static_assert(sizeof(Menu::Item) == 0x24, "Menu::Item layout");
static_assert(sizeof(Menu) == 0x190, "Menu layout");
