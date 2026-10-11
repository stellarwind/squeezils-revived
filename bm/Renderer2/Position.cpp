// Renderer::PositionWindowed (0x4764c0) and PositionFullscreen (0x4765d0).
#include "Renderer2/Renderer2.h"

int Renderer::PositionWindowed(int w, int h)
{
    SetLastError(0);
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    int dw, dh, dbpp;
    style |= WS_SYSMENU;
    style |= WS_MINIMIZEBOX;
    style |= WS_CAPTION;
    style |= WS_DLGFRAME;
    QueryDesktopMode(&dw, &dh, &dbpp);
    if (!SetWindowLong(hwnd, GWL_STYLE, style))
        GetLastError();
    RECT r;
    r.left = dw / 2 - w / 2;
    r.top = dh / 2 - h / 2;
    if (r.top <= 0)
        r.top += 20;
    if (r.left <= 0)
        r.left += 2;
    r.right = r.left + w;
    r.bottom = r.top + h;
    AdjustWindowRectEx(&r, style, 0, WS_EX_DLGMODALFRAME);
    SetWindowPos(hwnd, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_FRAMECHANGED | SWP_NOZORDER);
    return 1;
}

int Renderer::PositionFullscreen()
{
    SetLastError(0);
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if (!SetWindowLong(hwnd, GWL_STYLE, style & 0xff34ffff))
        GetLastError();
    SetWindowPos(hwnd, 0, 0, 0, 640, 480, SWP_FRAMECHANGED | SWP_NOZORDER);
    return 1;
}
