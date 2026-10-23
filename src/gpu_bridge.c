/* Redirect only retained Viewport::Render. Device/scene identities remain native.
 * Each target owns one DD surface reference until RM's device-destroy callback. */
#include "scene_gpu.h"
#include <stdio.h>
#include <string.h>

typedef struct DeviceTarget
{
    IDirect3DRMDevice *device;
    IDirectDrawSurface *surface;
} DeviceTarget;
static DeviceTarget targets[8];
static void install_geometry_tracking(void *object, BOOL builder)
{
    sqz_hook(object, builder ? "IDirect3DRMMeshBuilder" : "IDirect3DRMMesh");
}
BOOL sqz_gpu_enabled(void)
{
    char value[32] = "";
    GetEnvironmentVariableA("SQZ_RENDERER", value, sizeof(value));
    /* Ordinary SQZ.EXE launch uses GPU; native is an explicit comparison path. */
    return strcmp(value, "native") != 0;
}
static void __cdecl device_destroy(IDirect3DRMObject *object, void *ctx)
{
    (void)object;
    DeviceTarget *target = ctx;
    if (target->surface)
        target->surface->lpVtbl->Release(target->surface);
    memset(target, 0, sizeof(*target));
    BOOL any = FALSE;
    for (unsigned i = 0; i < 8; ++i)
        if (targets[i].device)
            any = TRUE;
    if (!any)
        sqz_gpu_shutdown();
}
void sqz_gpu_bind_device(IDirect3DRMDevice *device, IDirectDrawSurface *surface)
{
    if (!sqz_gpu_enabled() || !device || !surface)
        return;
    for (unsigned i = 0; i < 8; ++i)
        if (!targets[i].device)
        {
            targets[i] = (DeviceTarget){device, surface};
            surface->lpVtbl->AddRef(surface);
            if (FAILED(device->lpVtbl->AddDestroyCallback(device, device_destroy, &targets[i])))
                device_destroy(NULL, &targets[i]);
            return;
        }
}
HRESULT sqz_gpu_render(const IDirect3DRMViewportVtbl *native, IDirect3DRMViewport *viewport,
                       IDirect3DRMFrame *root)
{
    static BOOL shown, marked;
    IDirect3DRMDevice *device = NULL;
    IDirectDrawSurface *surface = NULL;
    if (SUCCEEDED(viewport->lpVtbl->GetDevice(viewport, &device)) && device)
    {
        for (unsigned i = 0; i < 8; ++i)
            if (targets[i].device == device)
                surface = targets[i].surface;
        device->lpVtbl->Release(device);
    }
    if (!sqz_gpu_enabled())
        return native->Render(viewport, root);
    SceneData scene = {0};
    BOOL ok = FALSE;
    char error[512] = "RM render target was not captured";
    if (surface)
    {
        sqz_geometry_install = install_geometry_tracking;
        ok = sqz_scene_extract_cached(&scene, viewport, root);
        if (!ok)
            snprintf(error, sizeof(error), "Scene extraction: %s", scene.error);
        else
            ok = sqz_gpu_draw(&scene, surface, error, sizeof(error));
    }
    if (ok && !marked)
    {
        sqz_platform_gpu_title();
        marked = TRUE;
    }
    if (!ok && !shown)
    {
        MessageBoxA(NULL, error, "SDL3 GPU renderer failed", MB_OK | MB_ICONERROR);
        shown = TRUE;
    }
    sqz_scene_free(&scene);
    return ok ? D3DRM_OK : E_FAIL;
}
