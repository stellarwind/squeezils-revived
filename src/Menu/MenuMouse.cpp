// Menu::OnMouseMove (0x406960): light the item under the pointer, unlight the others.
// 18 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
void PadDecl16();
void PadDecl17();
void PadDecl18();
#include "MenuD.h"

int MenuD::OnMouseMove(int x, int y)
{
    int moved = 0;
    for (int i = 0; i < itemCount; i++) {
        if (!items[i].highlighted && HitTest(i, x, y)) {
            DrawItem(i, 1);
            moved = 1;
        }
    }
    if (!moved)
        return 0;
    for (int j = 0; j < itemCount; j++) {
        if (!HitTest(j, x, y))
            DrawItem(j, 0);
    }
    return 0;
}
