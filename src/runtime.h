#ifndef SQZ_RUNTIME_H
#define SQZ_RUNTIME_H
#define COBJMACROS
#define DIRECTDRAW_VERSION 0x0700
#define DIRECT3D_VERSION 0x0700
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <d3drm.h>

typedef struct SqzInterface {
    const IID *iid;
    const char *name;
    size_t slots;
    const void *hooks;
} SqzInterface;
extern const SqzInterface sqz_interfaces[];
extern const size_t sqz_interface_count;
void sqz_log(const char *format, ...);
void *sqz_original(void *self);
void sqz_hook(void *self, const char *name);
void sqz_hook_iid(void *self, REFIID iid);
void sqz_forget(void *self);
HRESULT WINAPI sqz_create_rm(IDirect3DRM **out);
FARPROC __cdecl sqz_resolve(unsigned library, const char *name);
void sqz_platform_install(void);
BOOL sqz_platform_window(HWND hwnd);
HRESULT sqz_surface_blt(const IDirectDrawSurfaceVtbl *, IDirectDrawSurface *, RECT *,
                        IDirectDrawSurface *, RECT *, DWORD, DDBLTFX *);
HRESULT sqz_cooperative(const IDirectDrawVtbl *, IDirectDraw *, HWND, DWORD);
HRESULT sqz_display_mode(const IDirectDrawVtbl *, IDirectDraw *, DWORD, DWORD, DWORD);
HRESULT sqz_get_caps(const IDirectDrawVtbl *, IDirectDraw *, DDCAPS *, DDCAPS *);
void sqz_surface_created(IDirectDraw *, IDirectDrawSurface *);
HRESULT sqz_create_surface(const IDirectDrawVtbl *, IDirectDraw *, DDSURFACEDESC *, IDirectDrawSurface **, IUnknown *);
HRESULT sqz_release_dc(const IDirectDrawSurfaceVtbl *, IDirectDrawSurface *, HDC);
void sqz_surface_released(void *, ULONG);
#endif
