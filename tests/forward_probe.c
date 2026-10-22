#include "runtime.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int destroyed, moved;
static IDirect3DRMFrame *expected;
static void __cdecl on_destroy(IDirect3DRMObject *object, void *ctx)
{
    assert(IDirect3DRMObject_GetAppData(object) == 0x53515a);
    assert(ctx == &destroyed);
    ++destroyed;
}
static void __cdecl on_move(IDirect3DRMFrame *frame, void *ctx, D3DVALUE dt)
{
    assert(IDirect3DRMFrame_GetAppData(frame) == 0x53515a && ctx == &moved);
    assert(dt == 0.5f);
    ++moved;
}
static DWORD WINAPI references(void *arg)
{
    IDirect3DRMFrame *frame = arg;
    for (unsigned i = 0; i < 64; ++i) {
        IDirect3DRMFrame_AddRef(frame);
        IDirect3DRMFrame_Release(frame);
    }
    return 0;
}
int main(void)
{
    HRESULT (WINAPI *create_rm)(IDirect3DRM **);
    D3DCOLOR (WINAPI *color)(D3DVALUE, D3DVALUE, D3DVALUE);
    D3DVALUE (WINAPI *modulus)(D3DVECTOR *);
    int (__cdecl *sdl_version)(void);
    HMODULE dll = LoadLibraryA("d3drm.dll"), sdl;
    IDirectDraw *dd;
    IDirectDrawSurface *surface;
    IDirect3D *d3d;
    IDirect3DRM *rm;
    IDirect3DRMFrame *root, *frame, *element;
    IDirect3DRMFrameArray *children;
    IDirect3DRMMeshBuilder *builder;
    IDirect3DRMFace *face;
    IDirect3DRMMesh *mesh;
    IDirect3DRMMaterial *material;
    IUnknown *identity1, *identity2;
    DDSURFACEDESC desc = {0}, locked = {0};
    D3DVECTOR position, vector = {.x=3, .y=4, .z=0};
    HANDLE threads[2];
    assert(dll);
    create_rm = (void *)GetProcAddress(dll, "Direct3DRMCreate");
    if (getenv("SQZ_DROPIN_PROBE")) create_rm = Direct3DRMCreate;
    color = (void *)GetProcAddress(dll, "D3DRMCreateColorRGB");
    modulus = (void *)GetProcAddress(dll, "D3DRMVectorModulus");
    assert(create_rm && color && modulus);
    assert(color(1, 0, 0) == 0xffff0000);
    assert(fabsf(modulus(&vector) - 5) < 0.00001f);
    assert(SUCCEEDED(DirectDrawCreate(NULL, &dd, NULL)));
    assert(SUCCEEDED(IDirectDraw_QueryInterface(dd, &IID_IDirect3D, (void **)&d3d)));
    IDirect3D_Release(d3d);
    assert(SUCCEEDED(IDirectDraw_SetCooperativeLevel(dd, NULL, DDSCL_NORMAL)));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    desc.dwWidth = desc.dwHeight = 16;
    assert(SUCCEEDED(IDirectDraw_CreateSurface(dd, &desc, &surface, NULL)));
    locked.dwSize = sizeof(locked);
    assert(SUCCEEDED(IDirectDrawSurface_Lock(surface, NULL, &locked, DDLOCK_WAIT, NULL)));
    assert(locked.lpSurface && locked.lPitch);
    assert(SUCCEEDED(IDirectDrawSurface_Unlock(surface, locked.lpSurface)));
    DDCOLORKEY key = {0xf81f,0xf81f};
    assert(SUCCEEDED(IDirectDrawSurface_SetColorKey(surface,DDCKEY_SRCBLT,&key)));
    IDirectDrawSurface_Release(surface);
    IDirectDraw_Release(dd);
    assert(SUCCEEDED(create_rm(&rm)));
    BYTE pixels[256] = {0};
    D3DRMPALETTEENTRY palette[256] = {{255,0,255,0}};
    D3DRMIMAGE image = {.width=16,.height=16,.aspectx=1,.aspecty=1,.depth=8,
                        .bytes_per_line=16,.buffer1=pixels,.palette_size=256,.palette=palette};
    IDirect3DRMTexture *texture;
    assert(SUCCEEDED(IDirect3DRM_CreateTexture(rm,&image,&texture)));
    assert(IDirect3DRMTexture_GetImage(texture));
    assert(SUCCEEDED(IDirect3DRMTexture_SetDecalTransparency(texture,TRUE)));
    assert(SUCCEEDED(IDirect3DRMTexture_SetDecalTransparentColor(texture,0xffff00ff)));
    assert(IDirect3DRMTexture_GetDecalTransparency(texture));
    assert(texture->lpVtbl->GetDecalTransparentColor(texture) == 0xffff00ff);
    IDirect3DRMTexture_Release(texture);
    assert(SUCCEEDED(IDirect3DRM_CreateFrame(rm, NULL, &root)));
    assert(SUCCEEDED(IDirect3DRM_CreateFrame(rm, root, &frame)));
    expected = frame;
    assert(SUCCEEDED(IDirect3DRMFrame_SetAppData(frame, 0x53515a)));
    assert(SUCCEEDED(IDirect3DRMFrame_AddDestroyCallback(frame, on_destroy, &destroyed)));
    assert(SUCCEEDED(IDirect3DRMFrame_AddMoveCallback(frame, on_move, &moved)));
    assert(SUCCEEDED(IDirect3DRMFrame_Move(frame, 0.5f)) && moved == 1);
    assert(SUCCEEDED(IDirect3DRMFrame_SetPosition(frame, root, 1, 2, 3)));
    assert(SUCCEEDED(IDirect3DRMFrame_GetPosition(frame, root, &position)));
    assert(position.x == 1 && position.y == 2 && position.z == 3);
    assert(SUCCEEDED(IDirect3DRMFrame_QueryInterface(frame, &IID_IUnknown, (void **)&identity1)));
    assert(SUCCEEDED(IUnknown_QueryInterface(identity1, &IID_IUnknown, (void **)&identity2)));
    assert(identity1 == identity2);
    IUnknown_Release(identity1); IUnknown_Release(identity2);
    assert(SUCCEEDED(IDirect3DRMFrame_GetChildren(root, &children)));
    assert(IDirect3DRMFrameArray_GetSize(children) == 1);
    assert(SUCCEEDED(IDirect3DRMFrameArray_GetElement(children, 0, &element)));
    assert(SUCCEEDED(IDirect3DRMFrame_QueryInterface(element, &IID_IUnknown, (void **)&identity1)));
    assert(SUCCEEDED(IDirect3DRMFrame_QueryInterface(frame, &IID_IUnknown, (void **)&identity2)));
    assert(identity1 == identity2);
    IUnknown_Release(identity1); IUnknown_Release(identity2);
    IDirect3DRMFrame_Release(element); IDirect3DRMFrameArray_Release(children);
    assert(SUCCEEDED(IDirect3DRM_CreateMaterial(rm, 17.5f, &material)));
    assert(IDirect3DRMMaterial_GetPower(material) == 17.5f);
    IDirect3DRMMaterial_Release(material);
    assert(SUCCEEDED(IDirect3DRM_CreateMeshBuilder(rm, &builder)));
    assert(IDirect3DRMMeshBuilder_AddVertex(builder, 0, 0, 0) == 0);
    assert(IDirect3DRMMeshBuilder_AddVertex(builder, 1, 0, 0) == 1);
    assert(IDirect3DRMMeshBuilder_AddVertex(builder, 0, 1, 0) == 2);
    assert(IDirect3DRMMeshBuilder_AddNormal(builder, 0, 0, 1) == 0);
    assert(SUCCEEDED(IDirect3DRMMeshBuilder_CreateFace(builder, &face)));
    assert(SUCCEEDED(IDirect3DRMFace_AddVertexAndNormalIndexed(face, 0, 0)));
    assert(SUCCEEDED(IDirect3DRMFace_AddVertexAndNormalIndexed(face, 1, 0)));
    assert(SUCCEEDED(IDirect3DRMFace_AddVertexAndNormalIndexed(face, 2, 0)));
    IDirect3DRMFace_Release(face);
    assert(SUCCEEDED(IDirect3DRMMeshBuilder_CreateMesh(builder, &mesh)));
    assert(SUCCEEDED(IDirect3DRMFrame_AddVisual(frame, (IDirect3DRMVisual *)mesh)));
    IDirect3DRMMesh_Release(mesh); IDirect3DRMMeshBuilder_Release(builder);
    threads[0] = CreateThread(NULL, 0, references, frame, 0, NULL);
    threads[1] = CreateThread(NULL, 0, references, frame, 0, NULL);
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    CloseHandle(threads[0]); CloseHandle(threads[1]);
    assert(SUCCEEDED(IDirect3DRMFrame_DeleteChild(root, frame)));
    IDirect3DRMFrame_Release(frame);
    assert(destroyed == 1);
    IDirect3DRMFrame_Release(root); IDirect3DRM_Release(rm);
    sdl = LoadLibraryA("SDL3.dll"); assert(sdl);
    sdl_version = (void *)GetProcAddress(sdl, "SDL_GetVersion"); assert(sdl_version);
    assert(sdl_version() == 3004018);
    puts("PASS: DD surface lock, D3D QI, RM graph/mesh/face/float results, callbacks, identity, concurrent refs, SDL 3.4.18 x86");
    return 0;
}
