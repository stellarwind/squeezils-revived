// Bitmap members, SQZ.EXE 0x473760-0x474230.
#include "Bitmap2/Bitmap2.h"

Bitmap::Bitmap(Renderer *r)
{
    surface = 0;
    renderer = r;
    width = 0;
    height = 0;
}

Bitmap::~Bitmap()
{
    if (surface)
        surface->Release();
}

int Bitmap::CopyFrom(Bitmap *src)
{
    if (!surface || width != src->width || height != src->height) {
        if (surface)
            surface->Release();
        width = src->width;
        height = src->height;
        surface = CreateSurface();
    }
    if (!surface)
        return 0;
    if (!src->DrawToSurfaceAt(surface, 0, 0, 0, 0, 1))
        return 0;
    return SetColorKey(surface, 0xffffffff) == 0;
}

int Bitmap::CopyRectFrom(Bitmap *src, RECT srcRect, RECT dstRect)
{
    if (!surface) {
        width = src->width;
        height = src->height;
        surface = CreateSurface();
    }
    if (!surface)
        return 0;
    return src->DrawToSurface(surface, dstRect, srcRect) != 0;
}

LPDIRECTDRAWSURFACE Bitmap::GetSurface()
{
    return surface;
}

LPDIRECTDRAWSURFACE Bitmap::CreateSurfaceSized(int w, int h)
{
    LPDIRECTDRAWSURFACE s = 0;
    desc.dwSize = sizeof(desc);
    desc.dwWidth = w;
    desc.dwHeight = h;
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    HRESULT hr = renderer->ddraw->CreateSurface(&desc, &s, 0);
    if (hr != 0) {
        ReportError(hr);
        return 0;
    }
    width = w;
    height = h;
    surface = s;
    return s;
}

LPDIRECTDRAWSURFACE Bitmap::CreateSurface()
{
    LPDIRECTDRAWSURFACE s = 0;
    desc.dwSize = sizeof(desc);
    desc.dwWidth = width;
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    desc.dwHeight = height;
    HRESULT hr = renderer->ddraw->CreateSurface(&desc, &s, 0);
    if (hr != 0) {
        ReportError(hr);
        return 0;
    }
    return s;
}

int Bitmap::GetWidth()
{
    return width;
}

int Bitmap::GetHeight()
{
    return height;
}

void Bitmap::ReportError(long hr)
{
}
