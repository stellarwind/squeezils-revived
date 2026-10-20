// Renderer helpers: desktop query (0x474360, 0x4743b0), colour key (0x474230, 0x474310), message boxes (0x4754c0, 0x475510).
#include <stdio.h>
#include <stdarg.h>
#include "Renderer2/Renderer2.h"

void QueryDesktopMode(int *width, int *height, int *bpp)
{
    HDC dc = GetDC(0);
    *width = GetDeviceCaps(dc, HORZRES);
    *height = GetDeviceCaps(dc, VERTRES);
    *bpp = GetDeviceCaps(dc, BITSPIXEL);
    ReleaseDC(0, dc);
}

int CheckDesktopDepth(int *width, int *height)
{
    int bpp;
    QueryDesktopMode(width, height, &bpp);
    if (bpp >= 8 && bpp <= 16)
        return 1;
    MessageBox(0, "This application requires that your Display Properties\nare set to either 256 Colors or High Color (16-bit).\nPlease update your Display Properties with the Display Control Panel.", "Error", MB_OK);
    return 0;
}

DWORD __stdcall GetColorKeyValue(LPDIRECTDRAWSURFACE surf, COLORREF color)
{
    DWORD result = 0xffffffff;
    HDC dc;
    COLORREF saved;
    if (color != 0xffffffff) {
        if (surf->GetDC(&dc) == 0) {
            saved = GetPixel(dc, 0, 0);
            SetPixel(dc, 0, 0, color);
            surf->ReleaseDC(dc);
        }
    }
    DDSURFACEDESC desc;
    desc.dwSize = sizeof(desc);
    HRESULT hr;
    do {
        hr = surf->Lock(0, &desc, 0, 0);
    } while (hr == DDERR_WASSTILLDRAWING);
    if (hr == 0) {
        result = ((1 << desc.ddpfPixelFormat.dwRGBBitCount) - 1) & *(DWORD *)desc.lpSurface;
        surf->Unlock(0);
    }
    if (color != 0xffffffff) {
        if (surf->GetDC(&dc) == 0) {
            SetPixel(dc, 0, 0, saved);
            surf->ReleaseDC(dc);
        }
    }
    return result;
}

HRESULT __stdcall SetSourceColorKey(LPDIRECTDRAWSURFACE surf, COLORREF color)
{
    DDCOLORKEY key;
    key.dwColorSpaceLowValue = GetColorKeyValue(surf, color);
    key.dwColorSpaceHighValue = key.dwColorSpaceLowValue;
    return surf->SetColorKey(DDCKEY_SRCBLT, &key);
}

int Renderer::PrepareForDialog()
{
    return 0;
}

int Renderer::Error(const char *fmt, ...)
{
    char text[256];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    PrepareForDialog();
    return MessageBox(hwnd, text, "Note", MB_ICONINFORMATION);
}

int Renderer::FatalError(const char *fmt, ...)
{
    char text[256];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    PrepareForDialog();
    return MessageBox(hwnd, text, "Application Error", MB_RETRYCANCEL | MB_ICONERROR);
}
