// Pointer::MoveTo, SQZ.EXE 0x459150.
#include <windows.h>
#include "Mouse/Pointer.h"

struct PointerWindow { char pad[0x18]; HWND hwnd; };
extern PointerWindow *gPointerWindow; // 0x4c75dc

void Pointer::MoveTo(IPoint p)
{
    if (p.y < 0)
        p.y = 0;
    if (p.x < 0)
        p.x = 0;
    x = (short)p.x;
    y = (short)p.y;
    ClientToScreen(gPointerWindow->hwnd, (POINT *)&p);
    SetCursorPos(p.x, p.y);
}
