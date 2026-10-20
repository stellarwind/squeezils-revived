// 0x4781d0: ClientBitmap::Attach - memory DC + bitmap + 256-entry identity palette, painted with the palette as a 16x16 grid.
#include <windows.h>
#include <stdlib.h>
struct ClientBitmap {
    HDC     dc;         // 0x00
    HWND    window;     // 0x04
    HBITMAP bitmap;     // 0x08
    HPALETTE palette;   // 0x0c
    int     width;      // 0x10
    int     height;     // 0x14
    int Attach(HWND w, int cx, int cy);
};
int ClientBitmap::Attach(HWND w, int cx, int cy)
{
    window = w;
    width = cx;
    height = cy;
    int i;
    HDC screen = GetDC(w);
    dc = CreateCompatibleDC(screen);
    bitmap = CreateCompatibleBitmap(screen, width, height);
    SelectObject(dc, bitmap);
    LOGPALETTE *lp = (LOGPALETTE *)malloc(0x404);
    lp->palVersion = 0x300;
    lp->palNumEntries = 256;
    PALETTEENTRY *e = lp->palPalEntry;
    for (i = 0; i < 256; i++) {
        e->peRed = (BYTE)i;
        e++;
        e[-1].peGreen = 0;
        e[-1].peBlue = 0;
        e[-1].peFlags = 2;
    }
    palette = CreatePalette(lp);
    SelectPalette(dc, palette, 0);
    RealizePalette(dc);
    free(lp);
    RECT r;
    r.left = 0;
    r.top = 0;
    r.bottom = height - 1;
    r.right = width - 1;
    FillRect(dc, &r, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
    int cellW = width / 16;
    int cellH = height / 16;
    int base = 0;
    int y = 0;
    for (; base < 256; base += 16) {
        int col = 0;
        int nextY = y + cellH;
        int x = 0;
        for (; col < 16; col++) {
            int nextX = x + cellW;
            COLORREF color = ((base + col) & 0xffff) | 0x1000000;
            HBRUSH brush = CreateSolidBrush(color);
            HGDIOBJ oldBrush = SelectObject(dc, brush);
            HGDIOBJ oldPen = SelectObject(dc, CreatePen(PS_SOLID, 0, color));
            Rectangle(dc, x, y, nextX - 1, nextY - 1);
            SelectObject(dc, oldPen);
            DeleteObject(oldPen);
            SelectObject(dc, oldBrush);
            DeleteObject(brush);
            x = nextX;
        }
        y = nextY;
    }
    ReleaseDC(window, screen);
    return 1;
}
