// Renderer::UpdateScreenRect (0x475870), GetClientRectInWindow (0x474f70).
#include "Renderer2/Renderer2.h"

void Renderer::UpdateScreenRect()
{
    if (fullscreen) {
        if (doubleSize)
            SetRect(&screenRect, 0, 0, width * 2 - 1, height * 2 - 1);
        else
            SetRect(&screenRect, 0, 0, width - 1, height - 1);
        return;
    }
    POINT p = { 0, 0 };
    ClientToScreen(hwnd, &p);
    p.x += originX;
    p.y += originY;
    int w = doubleSize ? width * 2 : width;
    int h = doubleSize ? height * 2 : height;
    SetRect(&screenRect, p.x, p.y, p.x + w - 1, p.y + h - 1);
}

void Renderer::GetClientRectInWindow(LONG out[4])
{
    POINT tl, br;
    tl.x = screenRect.left;
    tl.y = screenRect.top;
    br.x = screenRect.right;
    br.y = screenRect.bottom;
    ScreenToClient(hwnd, &tl);
    ScreenToClient(hwnd, &br);
    out[0] = tl.x;
    out[1] = tl.y;
    out[2] = br.x;
    out[3] = br.y;
}
