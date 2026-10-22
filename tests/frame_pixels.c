/* Exercise the production CPU/DD boundary with padded and bottom-up rows. The
 * normal GPU/native oracle tests separately verify actual BGRA rendering. */
#include "../src/gpu_render.c"
#include <assert.h>
#include <stdarg.h>

void sqz_log(const char *format, ...)
{
    (void)format;
}

typedef struct PixelSurface
{
    IDirectDrawSurface iface;
    DDSURFACEDESC description;
    BYTE storage[40];
    unsigned locks, unlocks;
} PixelSurface;

static HRESULT WINAPI lock_pixels(IDirectDrawSurface *self, RECT *rect, DDSURFACEDESC *out,
                                  DWORD flags, HANDLE event)
{
    (void)rect;
    (void)flags;
    (void)event;
    PixelSurface *surface = (PixelSurface *)self;
    *out = surface->description;
    ++surface->locks;
    return DD_OK;
}
static HRESULT WINAPI unlock_pixels(IDirectDrawSurface *self, void *pixels)
{
    (void)pixels;
    ++((PixelSurface *)self)->unlocks;
    return DD_OK;
}

int main(void)
{
    IDirectDrawSurfaceVtbl table = {.Lock = lock_pixels, .Unlock = unlock_pixels};
    const DWORD formats[][4] = {{32, 0xff0000, 0xff00, 0xff},
                                {32, 0xff, 0xff00, 0xff0000},
                                {16, 0xf800, 0x7e0, 0x1f},
                                {16, 0x7c00, 0x3e0, 0x1f}};
    const BYTE colors[6][3] = {{255, 0, 0},     {0, 255, 0}, {0, 0, 255},
                               {255, 255, 255}, {0, 0, 0},   {255, 0, 255}};
    for (unsigned format = 0; format < 4; ++format)
        for (unsigned bottom_up = 0; bottom_up < 2; ++bottom_up)
        {
            PixelSurface surface = {.iface = {&table}};
            memset(surface.storage, 0xa5, sizeof(surface.storage));
            DDSURFACEDESC *desc = &surface.description;
            desc->dwSize = sizeof(*desc);
            desc->dwWidth = 3;
            desc->dwHeight = 2;
            desc->lPitch = bottom_up ? -20 : 20;
            desc->lpSurface = surface.storage + (bottom_up ? 20 : 0);
            DDPIXELFORMAT *pf = &desc->ddpfPixelFormat;
            pf->dwRGBBitCount = formats[format][0];
            pf->dwRBitMask = formats[format][1];
            pf->dwGBitMask = formats[format][2];
            pf->dwBBitMask = formats[format][3];
            unsigned bytes = pf->dwRGBBitCount / 8;
            for (unsigned i = 0; i < 6; ++i)
            {
                DWORD value = (colors[i][0] ? pf->dwRBitMask : 0) |
                              (colors[i][1] ? pf->dwGBitMask : 0) |
                              (colors[i][2] ? pf->dwBBitMask : 0);
                memcpy((BYTE *)desc->lpSurface + (ptrdiff_t)(i / 3) * desc->lPitch +
                           (i % 3) * bytes,
                       &value, bytes);
            }
            BYTE original[40];
            memcpy(original, surface.storage, sizeof(original));
            BYTE *bgra;
            unsigned width, height;
            assert(read_background(&surface.iface, &bgra, &width, &height));
            assert(width == 3 && height == 2);
            for (unsigned i = 0; i < 6; ++i)
            {
                assert(bgra[i * 4] == colors[i][2]);
                assert(bgra[i * 4 + 1] == colors[i][1]);
                assert(bgra[i * 4 + 2] == colors[i][0]);
            }
            assert(write_frame(&surface.iface, bgra, width, height));
            assert(!memcmp(original, surface.storage, sizeof(original)));
            assert(surface.locks == 2 && surface.unlocks == 2);
            free(bgra);
        }
    puts("PASS: BGRA rows and RGB565/RGB555/uncommon-mask fallbacks; padded/negative pitch "
         "preserved");
    return 0;
}
