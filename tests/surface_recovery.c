/* Exercise the production recovery helper with deterministic lost surfaces. */
#include "runtime.h"
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
typedef struct Surface { IDirectDrawSurface iface; unsigned blits, restores; BOOL fail_restore; } Surface;
static HRESULT WINAPI blt(IDirectDrawSurface *self, RECT *dst, IDirectDrawSurface *src, RECT *sr, DWORD flags, DDBLTFX *fx)
{
    Surface *surface = (Surface *)self;
    (void)dst; (void)src; (void)sr; (void)flags; (void)fx;
    return ++surface->blits == 1 ? DDERR_SURFACELOST : DD_OK;
}
static HRESULT WINAPI restore(IDirectDrawSurface *self)
{
    Surface *surface = (Surface *)self;
    ++surface->restores;
    return surface->fail_restore ? DDERR_WRONGMODE : DD_OK;
}
static HRESULT WINAPI lost(IDirectDrawSurface *self) { (void)self; return DDERR_SURFACELOST; }
void *sqz_original(void *self) { return *(void **)self; }
void sqz_hook(void *self, const char *name) { (void)self; (void)name; }
HRESULT WINAPI sqz_create_rm(IDirect3DRM **out) { (void)out; return E_NOTIMPL; }
int main(void)
{
    IDirectDrawSurfaceVtbl native = {.Blt=blt, .Restore=restore, .IsLost=lost};
    Surface destination = {{&native},0,0,FALSE}, source = {{&native},0,0,FALSE};
    assert(sqz_surface_blt(&native, &destination.iface, NULL, NULL, NULL, DDBLT_DEPTHFILL, NULL) == DD_OK);
    assert(destination.blits == 2 && destination.restores == 1);
    destination.blits = destination.restores = 0;
    destination.fail_restore = TRUE;
    assert(sqz_surface_blt(&native, &destination.iface, NULL, NULL, NULL, 0, NULL) == DDERR_SURFACELOST);
    assert(destination.blits == 1 && destination.restores == 1);
    destination.blits = destination.restores = 0;
    destination.fail_restore = FALSE;
    assert(sqz_surface_blt(&native, &destination.iface, NULL, &source.iface, NULL, 0, NULL) == DD_OK);
    assert(destination.blits == 2 && source.restores == 1);
    puts("PASS: production Blt recovery retries once, restores source, preserves failure");
    return 0;
}
