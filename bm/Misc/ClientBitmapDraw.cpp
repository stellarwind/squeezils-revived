// ClientBitmap::Draw, SQZ.EXE 0x4783e0.
#include <windows.h>
#include "Misc/Misc.h"

void ClientBitmap::Draw(int x, int y)
{
    HDC windowDC = GetDC((HWND)f4);
    BitBlt(windowDC, x, y, f10, f14, (HDC)f0, 0, 0, SRCCOPY);
    ReleaseDC((HWND)f4, windowDC);
}
