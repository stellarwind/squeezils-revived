/* Keep the original CPU warp at RGB565; compose its surface into a 32bpp frame. */
#include "runtime.h"
#include <stdint.h>
#include <string.h>

static BYTE *game_base;
static IDirectDrawSurface *warp_surface;
static BOOL active;
typedef int(__attribute__((thiscall)) * WarpLoad)(void *, const char *, int, int *);
static WarpLoad original_load;

static int __attribute__((thiscall)) load_warp(void *object, const char *path, int palette,
                                               int *format)
{
    int rgb565[6] = {16, 0, 0xf800, 0x7e0, 0x1f, 0};
    if (format && format[0] == 32 && !format[1])
        format = rgb565;
    return original_load(object, path, palette, format);
}

static uint32_t fingerprint(const BYTE *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < count; ++i)
        hash = (hash ^ bytes[i]) * 16777619u;
    return hash;
}

int sqz_warp_patch_image(BYTE *base, size_t size)
{
#ifdef SQZ_FIX_16BPP_WARP_BRIDGE
    DWORD protection, unused;
    BYTE call[5] = {0xe8, 0, 0, 0, 0};
    if (size < 0x76ae0 || fingerprint(base + 0x769a0, 0x140) != 0x8cfb4052 ||
        fingerprint(base + 0x767a0, 0x150) != 0x639ec97d)
        return -1;
    uint32_t relative = (uint32_t)(uintptr_t)load_warp - (uint32_t)(uintptr_t)(base + 0x76a51);
    memcpy(call + 1, &relative, 4);
    if (!VirtualProtect(base + 0x76a4c, 5, PAGE_EXECUTE_READWRITE, &protection))
        return -2;
    original_load = (WarpLoad)(base + 0x79090);
    game_base = base;
    memcpy(base + 0x76a4c, call, 5);
    FlushInstructionCache(GetCurrentProcess(), base + 0x76a4c, 5);
    VirtualProtect(base + 0x76a4c, 5, protection, &unused);
    active = TRUE;
    return 1;
#else
    (void)base;
    (void)size;
    (void)load_warp;
    (void)fingerprint;
    return 0;
#endif
}

HRESULT sqz_warp_create_surface(const IDirectDrawVtbl *original, IDirectDraw *dd,
                                DDSURFACEDESC *desc, IDirectDrawSurface **out, IUnknown *outer,
                                void *caller)
{
    DDSURFACEDESC mode = {.dwSize = sizeof(mode)}, adjusted;
    BOOL warp = active && caller == game_base + 0x768ae && desc &&
                SUCCEEDED(original->GetDisplayMode(dd, &mode)) &&
                mode.ddpfPixelFormat.dwRGBBitCount == 32;
    if (warp)
    {
        adjusted = *desc;
        adjusted.dwFlags |= DDSD_PIXELFORMAT;
        memset(&adjusted.ddpfPixelFormat, 0, sizeof(adjusted.ddpfPixelFormat));
        adjusted.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        adjusted.ddpfPixelFormat.dwFlags = DDPF_RGB;
        adjusted.ddpfPixelFormat.dwRGBBitCount = 16;
        adjusted.ddpfPixelFormat.dwRBitMask = 0xf800;
        adjusted.ddpfPixelFormat.dwGBitMask = 0x7e0;
        adjusted.ddpfPixelFormat.dwBBitMask = 0x1f;
        desc = &adjusted;
    }
    HRESULT result = sqz_create_surface(original, dd, desc, out, outer);
    if (warp && SUCCEEDED(result) && out)
    {
        warp_surface = *out;
        sqz_log("DIAG WARP rgb565 surface=%p size=%lux%lu", warp_surface, desc->dwWidth,
                desc->dwHeight);
    }
    return result;
}

void sqz_warp_surface_released(void *surface, ULONG refs)
{
    if (!refs && surface == warp_surface)
        warp_surface = NULL;
}

HRESULT sqz_warp_blt(const IDirectDrawSurfaceVtbl *original, IDirectDrawSurface *dst, RECT *dr,
                     IDirectDrawSurface *src, RECT *sr, DWORD flags, DDBLTFX *effects)
{
    if (active && src && src == warp_surface && flags == DDBLT_WAIT && !effects)
    {
        DDSURFACEDESC source = {.dwSize = sizeof(source)},
                      destination = {.dwSize = sizeof(destination)};
        if (SUCCEEDED(IDirectDrawSurface_GetSurfaceDesc(src, &source)) &&
            SUCCEEDED(original->GetSurfaceDesc(dst, &destination)) &&
            source.ddpfPixelFormat.dwRGBBitCount == 16 &&
            destination.ddpfPixelFormat.dwRGBBitCount == 32)
        {
            HDC source_dc = NULL, target_dc = NULL;
            HRESULT result = IDirectDrawSurface_GetDC(src, &source_dc);
            if (FAILED(result))
                return result;
            result = original->GetDC(dst, &target_dc);
            if (SUCCEEDED(result))
            {
                RECT s = sr ? *sr : (RECT){0, 0, source.dwWidth, source.dwHeight};
                RECT d = dr ? *dr : (RECT){0, 0, destination.dwWidth, destination.dwHeight};
                int saved = SaveDC(target_dc);
                SetStretchBltMode(target_dc, COLORONCOLOR);
                BOOL copied = StretchBlt(target_dc, d.left, d.top, d.right - d.left,
                                         d.bottom - d.top, source_dc, s.left, s.top,
                                         s.right - s.left, s.bottom - s.top, SRCCOPY);
                if (saved)
                    RestoreDC(target_dc, saved);
                HRESULT released = original->ReleaseDC(dst, target_dc);
                result = copied ? released : DDERR_GENERIC;
            }
            HRESULT released = IDirectDrawSurface_ReleaseDC(src, source_dc);
            return FAILED(result) ? result : released;
        }
    }
    return sqz_surface_blt(original, dst, dr, src, sr, flags, effects);
}
