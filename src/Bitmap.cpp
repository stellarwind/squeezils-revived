// Bitmap: see Bitmap.h. Reconstructed from SQZ.EXE 0x473760-0x474230.
#include "Bitmap.h"
#include "Image.h"
#include "Original.h"
#include <string.h>

typedef LPDIRECTDRAWSURFACE (__attribute__((thiscall)) *CreateSurfaceFn)(Bitmap *);
typedef BOOL (__attribute__((thiscall)) *DrawToSurfaceAtFn)(Bitmap *, LPDIRECTDRAWSURFACE, int, int, int, int, int);
typedef BOOL (__attribute__((thiscall)) *DrawToSurfaceFn)(Bitmap *, LPDIRECTDRAWSURFACE, int, int, int, int, int, int, int, int);

static const DWORD kBltColorKey = 0x1028000;   // DDBLT_WAIT | DDBLT_ROP | DDBLT_KEYSRC
static const DWORD kBltPlain    = 0x1020000;   // DDBLT_WAIT | DDBLT_ROP
static const HRESULT kSurfaceLost = (HRESULT)0x887601c2;       // DDERR_SURFACELOST
static const HRESULT kStillDrawing = (HRESULT)0x8876021c;      // DDERR_WASSTILLDRAWING

// @00473760
Bitmap::Bitmap(Renderer *r)
{
    vtbl = (void **)kBitmapVtable;
    surface = NULL;
    renderer = r;
    width = 0;
    height = 0;
}

// @00473890
Bitmap::~Bitmap()
{
    vtbl = (void **)kBitmapVtable;
    if (surface)
        surface->Release();
}

// @004738c0  A new offscreen surface of w x h in system memory; on success it becomes this bitmap's surface.
LPDIRECTDRAWSURFACE Bitmap::CreateSurfaceSized(int w, int h)
{
    LPDIRECTDRAWSURFACE s = NULL;
    desc.dwSize = sizeof desc;
    desc.dwWidth = w;
    desc.dwHeight = h;
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    HRESULT hr = renderer->ddraw->CreateSurface(&desc, &s, NULL);
    if (hr != DD_OK) {
        ReportError(hr);
        return NULL;
    }
    width = w;
    height = h;
    surface = s;
    return s;
}

// @00473940  The same for the size already stored; the caller keeps the surface.
LPDIRECTDRAWSURFACE Bitmap::CreateSurface()
{
    LPDIRECTDRAWSURFACE s = NULL;
    desc.dwSize = sizeof desc;
    desc.dwWidth = width;
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    desc.dwHeight = height;
    HRESULT hr = renderer->ddraw->CreateSurface(&desc, &s, NULL);
    if (hr != DD_OK) {
        ReportError(hr);
        return NULL;
    }
    return s;
}

// A blit that is retried while the hardware is busy and after a lost surface was restored. The restore is done on `dst`, the
// surface being drawn into. (A restore that succeeds ends the loop without blitting again, as in the original.)
BOOL Bitmap::BltRetry(LPDIRECTDRAWSURFACE dst, RECT *dstRect, LPDIRECTDRAWSURFACE src, RECT *srcRect, DWORD flags)
{
    DDBLTFX fx;
    memset(&fx, 0, sizeof fx);
    fx.dwSize = sizeof fx;
    fx.dwROP = SRCCOPY;
    HRESULT hr;
    do {
        hr = dst->Blt(dstRect, src, srcRect, flags, &fx);
        if (hr == DD_OK)
            break;
        if (hr == kSurfaceLost) {
            hr = dst->Restore();
            if (hr != DD_OK)
                break;
        }
    } while (hr == kStillDrawing);
    if (hr != DD_OK)
        ReportError(hr);
    return hr == DD_OK;
}

// @00473f70  Draws this bitmap into `dest`, colour keyed.
BOOL Bitmap::DrawToSurface(LPDIRECTDRAWSURFACE dest, int dstL, int dstT, int dstR, int dstB,
                           int srcL, int srcT, int srcR, int srcB)
{
    if (!surface || !dest)
        return FALSE;
    RECT d = { dstL, dstT, dstR, dstB }, s = { srcL, srcT, srcR, srcB };
    return BltRetry(dest, &d, surface, &s, kBltColorKey);
}

// @00474020  Draws all of this bitmap's size from (srcX, srcY) of itself to (destX, destY) of `dest`.
BOOL Bitmap::DrawToSurfaceAt(LPDIRECTDRAWSURFACE dest, int destX, int destY, int srcX, int srcY, int useColorKey)
{
    if (!surface || !dest)
        return FALSE;
    RECT s = { srcX, srcY, srcX + width, srcY + height };
    RECT d = { destX, destY, destX + width, destY + height };
    return BltRetry(dest, &d, surface, &s, useColorKey ? kBltColorKey : kBltPlain);
}

// @00474130  The same, into another bitmap's surface, always colour keyed.
BOOL Bitmap::DrawToBitmap(Bitmap *dest, int destX, int destY, int srcX, int srcY)
{
    if (!surface || !dest)
        return FALSE;
    LPDIRECTDRAWSURFACE d = dest->surface;
    RECT s = { srcX, srcY, srcX + width, srcY + height };
    RECT r = { destX, destY, destX + width, destY + height };
    return BltRetry(d, &r, surface, &s, kBltColorKey);
}

// @00473e80  Fills this bitmap with the width x height area of `src` at (x, y).
BOOL Bitmap::CaptureFrom(LPDIRECTDRAWSURFACE src, int x, int y)
{
    if (!width || !height || !surface)
        return FALSE;
    RECT s = { x, y, x + width, y + height };
    RECT d = { 0, 0, width, height };
    return BltRetry(surface, &d, src, &s, kBltPlain);
}

// @00473780  Becomes a copy of `src`: the surface is recreated when the size differs.
BOOL Bitmap::CopyFrom(Bitmap *src)
{
    if (!surface || width != src->width || height != src->height) {
        if (surface)
            surface->Release();
        width = src->width;
        height = src->height;
        surface = ((CreateSurfaceFn)vtbl[SLOT_CREATE])(this);
    }
    if (!surface)
        return FALSE;
    if (!((DrawToSurfaceAtFn)src->vtbl[SLOT_DRAW_TO_SURFACE_AT])(src, surface, 0, 0, 0, 0, 1))
        return FALSE;
    return SetSourceColorKey(surface, 0xffffffff) == DD_OK;
}

// @00473810  Copies a rectangle of `src` into a rectangle of this bitmap (creating the surface at src's size when there is none).
BOOL Bitmap::CopyRectFrom(Bitmap *src, int srcL, int srcT, int srcR, int srcB, int dstL, int dstT, int dstR, int dstB)
{
    if (!surface) {
        width = src->width;
        height = src->height;
        surface = ((CreateSurfaceFn)vtbl[SLOT_CREATE])(this);
    }
    if (!surface)
        return FALSE;
    return ((DrawToSurfaceFn)src->vtbl[SLOT_DRAW_TO_SURFACE])(src, surface, dstL, dstT, dstR, dstB,
                                                             srcL, srcT, srcR, srcB) != 0;
}

// A ReadFile that asks the user what to do when it fails: Retry reads again, Cancel gives up (returns FALSE). After a Retry
// the original keeps re-reading (it tests the answer again after a successful read), which is kept.
BOOL Bitmap::ReadRetry(HANDLE file, void *buffer, DWORD size, const char *path)
{
    DWORD got;
    int answer = 1;
    do {
        while (!ReadFile(file, buffer, size, &got, NULL)) {
            answer = renderer->FatalError("Could not read file: %s ", path);
            if (answer == 2)
                return FALSE;
        }
    } while (answer == 4);
    return TRUE;
}

// @004739d0  Loads an 8 bit Windows bitmap into a new surface of this object (made with the virtual CreateSurface) in the
// display's pixel format, with the colour key set to black. Returns the surface, or NULL after reporting the problem. The
// file is expected to be laid out as header, palette, pixels back to back (the offset to the pixels is not used).
void (*BitmapLoadTrace)(const char *path, int ok) = NULL;

LPDIRECTDRAWSURFACE Bitmap::Load(const char *path)
{
    LPDIRECTDRAWSURFACE s = LoadFile(path);
    if (BitmapLoadTrace) BitmapLoadTrace(path, s != NULL);
    return s;
}

LPDIRECTDRAWSURFACE Bitmap::LoadFile(const char *path)
{
    // open (CreateFile gives INVALID_HANDLE_VALUE, not 0, for a missing file, so the dialog below never shows; the first
    // ReadFile then fails instead)
    HANDLE file = 0;
    int answer = 1;
    while (file == 0 || answer == 4) {
        file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
        if (file == 0) {
            answer = renderer->FatalError("Could not read file: %s ", path);
            if (answer == 2)
                return NULL;
        }
    }

    unsigned char fileHeader[14];                        // BITMAPFILEHEADER
    if (!ReadRetry(file, fileHeader, sizeof fileHeader, path)) { CloseHandle(file); return NULL; }
    BITMAPINFOHEADER header;
    if (!ReadRetry(file, &header, sizeof header, path)) { CloseHandle(file); return NULL; }

    // the info block: the header followed by the palette (4 bytes an entry for 1 << bit count entries)
    DWORD paletteBytes = (1u << (header.biBitCount & 0x1f)) * 4;
    DWORD infoBytes = sizeof header + (paletteBytes > 1024 ? paletteBytes : 1024);   // (always room for 256 entries)
    unsigned char *info = (unsigned char *)Lib_Malloc(infoBytes);
    if (!info) {
        renderer->FatalError("Can't allocate memory for\n%s", path);
        CloseHandle(file);
        return NULL;
    }
    memset(info, 0, infoBytes);
    memcpy(info, &header, sizeof header);
    if (!ReadRetry(file, info + sizeof header, paletteBytes, path)) {
        Lib_Free(info);
        CloseHandle(file);
        return NULL;
    }

    int fileSize, pixelOffset;
    memcpy(&fileSize, fileHeader + 2, 4);
    memcpy(&pixelOffset, fileHeader + 10, 4);
    DWORD pixelBytes = (DWORD)(fileSize - pixelOffset);
    unsigned char *pixels = (unsigned char *)Lib_Malloc(pixelBytes);
    if (!pixels) {
        renderer->FatalError("Can't allocate memory for\n%s", path);
        Lib_Free(info);
        CloseHandle(file);
        return NULL;
    }
    if (!ReadRetry(file, pixels, pixelBytes, path)) {
        Lib_Free(pixels);
        Lib_Free(info);
        CloseHandle(file);
        return NULL;
    }

    width = (short)header.biWidth;
    height = (short)header.biHeight;
    surface = ((CreateSurfaceFn)vtbl[SLOT_CREATE])(this);
    if (!surface) {
        Lib_Free(pixels);
        Lib_Free(info);
        CloseHandle(file);
        return NULL;
    }
    surface->Restore();

    DDSURFACEDESC lockDesc;
    memset(&lockDesc, 0, sizeof lockDesc);
    lockDesc.dwSize = sizeof lockDesc;
    if (surface->Lock(NULL, &lockDesc, DDLOCK_WAIT, NULL) != DD_OK) {
        renderer->FatalError("Can't lock surface for\n%s", path);
        surface->Release();
        surface = NULL;
        Lib_Free(pixels);
        Lib_Free(info);
        CloseHandle(file);
        return NULL;
    }

    DisplayFormat fmt;
    renderer->GetPixelFormat((int *)&fmt);

    // the palette as three planes (red, green, blue) from the file's blue-green-red-unused entries
    unsigned char planes[768];
    const unsigned char *entry = info + sizeof header;
    for (int i = 0; i < 256; i++, entry += 4) {
        planes[i] = entry[2];
        planes[0x100 + i] = entry[1];
        planes[0x200 + i] = entry[0];
    }

    // bitmaps are stored bottom row first, each row padded to 4 bytes
    int stride = (width + 3) & ~3;
    for (int y = 0; y < height; y++)
        ImageConvertRow((unsigned char *)lockDesc.lpSurface + (size_t)y * lockDesc.lPitch, &fmt,
                        pixels + (size_t)(height - 1 - y) * stride, planes, width);

    surface->Unlock(lockDesc.lpSurface);
    Lib_Free(pixels);
    Lib_Free(info);
    CloseHandle(file);
    SetSourceColorKey(surface, 0xffffffff);
    return surface;
}
