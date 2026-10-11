// Bitmap blits, SQZ.EXE 0x473e80-0x474230. The retry loop is inlined into each.
#include <string.h>
#include "Bitmap2/Bitmap2.h"

#define BLT_RETRY(DST, DRECT, SRC, SRECT, FLAGS)                                       \
    for (;;) {                                                                          \
        hr = (DST)->Blt(DRECT, SRC, SRECT, FLAGS, &fx);                                 \
        if (hr == 0)                                                                    \
            break;                                                                      \
        if (hr == 0x887601c2) {                                                         \
            hr = (DST)->Restore();                                                      \
            if (hr != 0)                                                                \
                goto report;                                                            \
        }                                                                               \
        if (hr == 0x8876021c)                                                           \
            continue;                                                                   \
        if (hr == 0)                                                                    \
            break;                                                                      \
report:                                                                                 \
        ReportError(hr);                                                                \
        break;                                                                          \
    }

int Bitmap::CaptureFrom(LPDIRECTDRAWSURFACE src, int x, int y)
{
    if (!width || !height || !surface)
        return 0;
    RECT d, s;
    d.left = 0;
    d.top = 0;
    d.right = width;
    d.bottom = height;
    s.left = x;
    s.top = y;
    s.right = x + width;
    s.bottom = y + height;
    DDBLTFX fx;
    memset(&fx, 0, sizeof(fx));
    fx.dwSize = sizeof(fx);
    fx.dwROP = SRCCOPY;
    HRESULT hr;
    BLT_RETRY(surface, &d, src, &s, 0x1020000)
    return hr == 0;
}

int Bitmap::DrawToSurface(LPDIRECTDRAWSURFACE dest, RECT dstRect, RECT srcRect)
{
    if (!surface || !dest)
        return 0;
    DDBLTFX fx;
    memset(&fx, 0, sizeof(fx));
    fx.dwSize = sizeof(fx);
    fx.dwROP = SRCCOPY;
    HRESULT hr;
    BLT_RETRY(dest, &dstRect, surface, &srcRect, 0x1028000)
    return hr == 0;
}

int Bitmap::DrawToSurfaceAt(LPDIRECTDRAWSURFACE dest, int x, int y, int sx, int sy, int key)
{
    if (!surface || !dest)
        return 0;
    RECT s, d;
    s.left = sx;
    s.top = sy;
    s.bottom = sy + height;
    s.right = sx + width;
    d.left = x;
    d.top = y;
    d.right = x + width;
    d.bottom = y + height;
    DDBLTFX fx;
    memset(&fx, 0, sizeof(fx));
    fx.dwSize = sizeof(fx);
    fx.dwROP = SRCCOPY;
    DWORD flags = 0x1020000;
    if (key)
        flags = 0x1028000;
    HRESULT hr;
    BLT_RETRY(dest, &d, surface, &s, flags)
    return hr == 0;
}

int Bitmap::DrawToBitmap(Bitmap *other, int x, int y, int sx, int sy)
{
    if (!surface || !other)
        return 0;
    LPDIRECTDRAWSURFACE dest = other->surface;
    RECT s, d;
    s.left = sx;
    s.top = sy;
    s.bottom = sy + height;
    s.right = sx + width;
    d.left = x;
    d.top = y;
    d.right = x + width;
    d.bottom = y + height;
    DDBLTFX fx;
    memset(&fx, 0, sizeof(fx));
    fx.dwSize = sizeof(fx);
    fx.dwROP = SRCCOPY;
    HRESULT hr;
    BLT_RETRY(dest, &d, surface, &s, 0x1028000)
    return hr == 0;
}
