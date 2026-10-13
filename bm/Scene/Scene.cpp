// Scene members, SQZ.EXE 0x476720-0x477400.
#include <string.h>
#include "Scene/Scene.h"

// 24 spare declarations: the compiler's register choices follow the symbol count (tools/flagsearch/padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
void PadDecl16();
void PadDecl17();
void PadDecl18();
void PadDecl19();
void PadDecl20();
void PadDecl21();
void PadDecl22();
void PadDecl23();
void PadDecl24();

Scene::Scene()
{
    renderer = 0;
    warpSurface = 0;
    lightsFrame = 0;
    sceneChild = 0;
    warp = 0;
    warpWidth = 0;
    warpHeight = 0;
}

Scene::~Scene()
{
    DeleteSceneChild();
    if (lightsFrame) {
        lightsFrame->Release();
        lightsFrame = 0;
    }
    if (warpSurface) {
        warpSurface->Release();
        warpSurface = 0;
    }
    WarpSky *w = warp;
    renderer = 0;
    if (w) {
        delete w;
        warp = 0;
    }
}

int Scene::Render()
{
    HRESULT hr = renderer->viewport->Render(renderer->sceneFrame);
    if (hr)
        renderer->Error("Render failed.\n%s", HResultToString(hr));
    return hr == 0;
}

void Scene::SetCameraTransform(const float *m)
{
    SetFrameTransform(GetCameraFrame(), m);
    if (warp)
        warp->Update(m);
}

void Scene::SetViewParams(float nearPlane, float farPlane, float fovDegrees)
{
    WarpSky *w = warp;
    if (w) {
        w->fov = fovDegrees;
        w->fovScaled = (float)w->height / (float)w->width * fovDegrees;
        SkySetFov(w->fov, w->fovScaled);
    }
    if (renderer)
        ((RendererV *)renderer)->SetViewParams(nearPlane, farPlane, fovDegrees);
}

LPDIRECT3DRMFRAME Scene::CreateSceneFrame()
{
    LPDIRECT3DRMFRAME frame;
    HRESULT hr = renderer->d3drm->CreateFrame(sceneChild, &frame);
    if (hr)
        renderer->Error("CreateFrame failed.\n%s", HResultToString(hr));
    frame->Release();
    return frame;
}

LPDIRECT3DRMFRAME Scene::CreateFrameUnder(LPDIRECT3DRMFRAME parent)
{
    LPDIRECT3DRMFRAME frame;
    HRESULT hr = renderer->d3drm->CreateFrame(parent, &frame);
    if (hr)
        renderer->Error("Render failed.\n%s", HResultToString(hr));
    if (parent)
        frame->Release();
    return frame;
}

void Scene::SetFrameTransform(LPDIRECT3DRMFRAME frame, const float *m)
{
    HRESULT hr = frame->SetPosition(sceneChild, m[12], m[13], m[14]);
    if (hr)
        renderer->Error("SetPosition failed.\n%s", HResultToString(hr));
    hr = frame->SetOrientation(sceneChild, m[8], m[9], m[10], m[4], m[5], m[6]);
    if (hr)
        renderer->Error("SetOrientation failed.\n%s", HResultToString(hr));
}

int Scene::SetSceneChild(LPDIRECT3DRMFRAME frame)
{
    if (!DeleteSceneChild())
        return 0;
    sceneChild = frame;
    HRESULT hr = renderer->sceneFrame->AddChild(frame);
    sceneChild->Release();
    if (hr) {
        renderer->Error("AddChild failed.\n%s", HResultToString(hr));
        return 0;
    }
    return 1;
}

int Scene::DeleteSceneChild()
{
    if (sceneChild) {
        HRESULT hr = renderer->sceneFrame->DeleteChild(sceneChild);
        if (hr) {
            renderer->Error("DeleteChild failed.\n%s", HResultToString(hr));
            return 0;
        }
        sceneChild = 0;
    }
    return 1;
}

LPDIRECT3DRMFRAME Scene::GetSceneChild()
{
    return sceneChild;
}

LPDIRECT3DRMFRAME Scene::GetCameraFrame()
{
    return renderer->cameraFrame;
}

LPDIRECT3DRMFRAME __stdcall GetFrameParent(LPDIRECT3DRMFRAME frame)
{
    LPDIRECT3DRMFRAME parent;
    frame->GetParent(&parent);
    parent->Release();
    return parent;
}

int __stdcall DestroyFrame(LPDIRECT3DRMFRAME frame)
{
    LPDIRECT3DRMFRAME parent;
    frame->GetParent(&parent);
    if (parent) {
        parent->DeleteChild(frame);
        parent->Release();
    } else
        frame->Release();
    return 1;
}

int Scene::EnsureWarpSurface()
{
    LPDIRECTDRAWSURFACE old = warpSurface;
    if (old) {
        if ((renderer->doubleSize ? renderer->width * 2 : renderer->width) / 2 == warpWidth
            && (renderer->doubleSize ? renderer->height * 2 : renderer->height) / 2 == warpHeight)
            return 1;
        old->Release();
        warpSurface = 0;
    }
    warpWidth = (renderer->doubleSize ? renderer->width * 2 : renderer->width) / 2;
    warpHeight = (renderer->doubleSize ? renderer->height * 2 : renderer->height) / 2;
    if (warp)
        warp->Resize(warpWidth, warpHeight);
    DDSURFACEDESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwWidth = warpWidth;
    desc.dwHeight = warpHeight;
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    HRESULT hr = renderer->ddraw->CreateSurface(&desc, &warpSurface, 0);
    if (hr < 0) {
        renderer->FatalError("CreateSurface failed (warp).\n%s", HResultToString(hr));
        return 0;
    }
    return 1;
}

int Scene::Init(Renderer *r)
{
    renderer = r;
    EnsureWarpSurface();
    HRESULT hr = renderer->d3drm->CreateFrame(renderer->sceneFrame, &lightsFrame);
    if (hr) {
        renderer->Error("CreateFrame failed (lights).\n%s", HResultToString(hr));
        return 0;
    }
    hr = renderer->d3drm->CreateFrame(renderer->sceneFrame, &sceneChild);
    if (hr) {
        renderer->Error("CreateFrame failed (scene).\n%s", HResultToString(hr));
        return 0;
    }
    sceneChild->Release();
    warp = 0;
    return 1;
}

int Scene::Present()
{
    if (renderer->field_078)
        return 1;
    RECT src, dst;
    src.left = 0;
    src.top = 0;
    src.right = renderer->width;
    src.bottom = renderer->height;
    dst.left = renderer->screenRect.left;
    dst.top = renderer->screenRect.top;
    dst.right = dst.left + (renderer->doubleSize ? src.right * 2 : src.right);
    dst.bottom = dst.top + (renderer->doubleSize ? src.bottom * 2 : src.bottom);
    if (renderer->primary->IsLost() < 0)
        renderer->primary->Restore();
    HRESULT hr = renderer->primary->Blt(&dst, renderer->backBuffer, &src, DDBLT_WAIT, 0);
    if (hr < 0)
        renderer->Error("Blt failed from back to front.\n%s", HResultToString(hr));
    return hr >= 0;
}

long __stdcall AddVisualToFrame(LPDIRECT3DRMFRAME frame, LPDIRECT3DRMVISUAL visual)
{
    return frame->AddVisual(visual);
}

long __stdcall AddChildToFrame(LPDIRECT3DRMFRAME frame, LPDIRECT3DRMFRAME child)
{
    return frame->AddChild(child);
}

long __stdcall DeleteChildFromFrame(LPDIRECT3DRMFRAME frame, LPDIRECT3DRMFRAME child)
{
    return frame->DeleteChild(child);
}

void __stdcall ApplyFrameTransform(LPDIRECT3DRMFRAME frame, LPDIRECT3DRMFRAME ref, const float *m)
{
    frame->SetPosition(ref, m[12], m[13], m[14]);
    frame->SetOrientation(ref, m[8], m[9], m[10], m[4], m[5], m[6]);
}

LPDIRECT3DRMFRAME __stdcall SetFrameDecalSize(LPDIRECT3DRMFRAME frame, float width, float height)
{
    LPDIRECT3DRMVISUALARRAY visuals;
    frame->GetVisuals(&visuals);
    if (visuals->GetSize() != 1)
        return 0;
    LPDIRECT3DRMVISUAL visual;
    visuals->GetElement(0, &visual);
    visual->Release();
    ((LPDIRECT3DRMTEXTURE)visual)->SetDecalSize(width, height);
    visuals->Release();
    return frame;
}

int Scene::BeginFrame()
{
    DDBLTFX fx;
    fx.dwSize = sizeof(fx);
    fx.dwFillDepth = 0xffffffff;
    HRESULT hr = renderer->depth->Blt(0, 0, 0, 0x3000000, &fx);
    if (hr < 0)
        renderer->Error("Blt failed (Z buffer clear).\n%s", HResultToString(hr));
    if (warp == 0) {
        DDBLTFX fx2;
        fx2.dwSize = sizeof(fx2);
        fx2.dwFillColor = 0;
        hr = renderer->backBuffer->Blt(0, 0, 0, 0x1000400, &fx2);
        if (hr < 0)
            renderer->Error("Blt failed for clear (back).\n%s", HResultToString(hr));
    } else {
        EnsureWarpSurface();
        {
        DDSURFACEDESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc);
        hr = warpSurface->Lock(0, &desc, DDLOCK_WAIT, 0);
        if (hr < 0) {
            renderer->Error("Lock failed (warp).\n%s", HResultToString(hr));
            return 0;
        }
        warp->SetOutput(desc.lpSurface);
        SkyDraw();
        hr = warpSurface->Unlock(desc.lpSurface);
        if (hr < 0)
            renderer->Error("Unlock failed (warp).\n%s", HResultToString(hr));
        }
        RECT src, dst;
        src.left = 0;
        src.top = 0;
        src.right = warpWidth;
        src.bottom = warpHeight;
        dst.left = 0;
        dst.top = 0;
        dst.right = renderer->width;
        dst.bottom = renderer->height;
        hr = renderer->backBuffer->Blt(&dst, warpSurface, &src, 0x1000000, 0);
        if (hr < 0) {
            renderer->Error("Blt failed (warp).\n%s", HResultToString(hr));
            return 0;
        }
    }
    return 1;
}

int Scene::LoadWarp(const char *name)
{
    if (warp == 0) {
        warp = new WarpSky;
        if (warp == 0)
            return 0;
        warp->Init(warpHeight, warpWidth, renderer->fovDegrees, 1);
    }
    int format[6];
    {
        int tmp[6];
        memcpy(format, renderer->GetPixelFormat(tmp), sizeof(format));
    }
    unsigned char planes[768];
    if (!warp->Load(name, planes, format)) {
        if (warp) {
            delete warp;
        }
        warp = 0;
        return 0;
    }
    unsigned char rgba[1024];
    for (int i = 0; i < 256; i++) {
        rgba[i * 4] = planes[i];
        rgba[i * 4 + 1] = planes[256 + i];
        rgba[i * 4 + 2] = planes[512 + i];
    }
    renderer->SetPaletteEntries(10, 0x20, rgba + 40);
    return 1;
}
