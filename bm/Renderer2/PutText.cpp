// Renderer::PutText (0x4753d0): white text on the window at (x, y), clipped to its own box.
#include <string.h>
#include "Renderer2/Renderer2.h"

void Renderer::PutText(HFONT font, int x, int y, const char *text)
{
    HDC dc = GetDC(hwnd);
    HGDIOBJ old;
    if (font)
        old = SelectObject(dc, font);
    SIZE size;
    GetTextExtentPoint32(dc, text, strlen(text), &size);
    RECT box;
    GetClientRect(hwnd, &box);
    box.left = x;
    box.top = y;
    box.bottom = y + size.cy;
    box.right = x + size.cx;
    SetTextColor(dc, 0xffffff);
    SetBkMode(dc, TRANSPARENT);
    ExtTextOut(dc, x, y, ETO_CLIPPED, &box, text, strlen(text), 0);
    if (font)
        SelectObject(dc, old);
    ReleaseDC(hwnd, dc);
}
