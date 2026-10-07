// Scene: see Scene.h. Reconstructed from SQZ.EXE 0x476720-0x477400.
#include "Scene.h"
#include <string.h>

extern const char *HResultToString(HRESULT hr);          // @00478430
// Warp backdrop object (not reconstructed yet), original functions:
extern void Warp_Destroy(void *warp);                    // @00478cb0 thiscall
extern void Warp_Resize(void *warp, int w, int h);       // @00478dc0 thiscall
extern void Warp_Render(void *warp, void *bits);         // @00478da0 thiscall
extern void Operator_Delete(void *p);                    // @0048e9f0 cdecl
extern void Unknown_48C710();                            // @0048c710 called after the warp is drawn (FPU/CRT state helper, no args)

// @00476720
Scene::Scene()
{
    renderer = NULL; warpSurface = NULL; lightsFrame = NULL; sceneChild = NULL;
    warp = NULL; warpWidth = 0; warpHeight = 0;
}

// @00476740
Scene::~Scene()
{
    DeleteSceneChild();
    if (lightsFrame) { lightsFrame->Release(); lightsFrame = NULL; }
    if (warpSurface) { warpSurface->Release(); warpSurface = NULL; }
    renderer = NULL;
    if (warp) { Warp_Destroy(warp); Operator_Delete(warp); warp = NULL; }
}

// @004767a0
// (Re)creates the warp surface when the render size changed. The warp is rendered at half resolution.
BOOL Scene::EnsureWarpSurface()
{
    int scale = renderer->doubleSize ? 2 : 1;
    if (warpSurface) {
        if ((renderer->width * scale) / 2 == warpWidth && (renderer->height * scale) / 2 == warpHeight)
            return TRUE;
        warpSurface->Release();
        warpSurface = NULL;
    }
    warpWidth  = (renderer->width  * scale) / 2;
    warpHeight = (renderer->height * scale) / 2;
    if (warp)
        Warp_Resize(warp, warpWidth, warpHeight);

    DDSURFACEDESC desc;
    memset(&desc, 0, sizeof desc);
    desc.dwSize = sizeof desc;
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    desc.dwHeight = warpHeight;
    desc.dwWidth = warpWidth;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    HRESULT hr = renderer->ddraw->CreateSurface(&desc, &warpSurface, NULL);
    if (hr >= 0)
        return TRUE;
    renderer->Error("CreateSurface failed (warp).\n%s", HResultToString(hr));
    return FALSE;
}

// @004768f0
BOOL Scene::Init(Renderer *r)
{
    HRESULT hr;
    renderer = r;
    EnsureWarpSurface();
    if ((hr = renderer->d3drm->CreateFrame(renderer->sceneFrame, &lightsFrame)) != 0) {
        renderer->Error("CreateFrame failed (lights).\n%s", HResultToString(hr));
        return FALSE;
    }
    if ((hr = renderer->d3drm->CreateFrame(renderer->sceneFrame, &sceneChild)) != 0) {
        renderer->Error("CreateFrame failed (scene).\n%s", HResultToString(hr));
        return FALSE;
    }
    sceneChild->Release();   // the parent keeps the frame alive; we only hold a plain pointer
    warp = NULL;
    return TRUE;
}

// @00476af0
// Clears the z-buffer, then either clears the back buffer to black or draws the warp backdrop into it.
BOOL Scene::BeginFrame()
{
    HRESULT hr;
    DDBLTFX fx;

    memset(&fx, 0, sizeof fx);
    fx.dwSize = sizeof fx;
    fx.dwFillDepth = 0xffffffff;
    hr = renderer->depth->Blt(NULL, NULL, NULL, DDBLT_DEPTHFILL | DDBLT_WAIT, &fx);
    if (hr < 0)
        renderer->Error("Blt failed (Z buffer clear).\n%s", HResultToString(hr));

    if (warp == NULL) {
        memset(&fx, 0, sizeof fx);
        fx.dwSize = sizeof fx;
        fx.dwFillColor = 0;
        hr = renderer->backBuffer->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
        if (hr < 0)
            renderer->Error("Blt failed for clear (back).\n%s", HResultToString(hr));
        return TRUE;
    }

    EnsureWarpSurface();
    DDSURFACEDESC desc;
    memset(&desc, 0, sizeof desc);
    desc.dwSize = sizeof desc;
    hr = warpSurface->Lock(NULL, &desc, DDLOCK_WAIT, NULL);
    if (hr < 0) {
        renderer->Error("Lock failed (warp).\n%s", HResultToString(hr));
        return FALSE;
    }
    Warp_Render(warp, desc.lpSurface);
    Unknown_48C710();
    hr = warpSurface->Unlock(desc.lpSurface);
    if (hr < 0)
        renderer->Error("Unlock failed (warp).\n%s", HResultToString(hr));

    RECT src = { 0, 0, warpWidth, warpHeight };
    RECT dst = { 0, 0, renderer->width, renderer->height };
    hr = renderer->backBuffer->Blt(&dst, warpSurface, &src, DDBLT_WAIT, NULL);   // stretched up to full size
    if (hr < 0) {
        renderer->Error("Blt failed (warp).\n%s", HResultToString(hr));
        return FALSE;
    }
    return TRUE;
}

// @00476cd0
BOOL Scene::Render()
{
    HRESULT hr = renderer->viewport->Render(renderer->sceneFrame);
    if (hr != 0)
        renderer->Error("Render failed.\n%s", HResultToString(hr));
    return hr == 0;
}

// @00476d20
// Copies the back buffer to the primary surface (scaled 2x when doubleSize). field_078 != 0 means the
// primary surface is presented some other way and nothing is done here.
BOOL Scene::Present()
{
    if (renderer->field_078 != 0)
        return TRUE;

    RECT src = { 0, 0, renderer->width, renderer->height };
    int scale = renderer->doubleSize ? 2 : 1;
    RECT dst;
    dst.left   = renderer->screenRect.left;
    dst.top    = renderer->screenRect.top;
    dst.right  = dst.left + renderer->width  * scale;
    dst.bottom = dst.top  + renderer->height * scale;

    if (renderer->primary->IsLost() < 0)
        renderer->primary->Restore();
    HRESULT hr = renderer->primary->Blt(&dst, renderer->backBuffer, &src, DDBLT_WAIT, NULL);
    if (hr < 0)
        renderer->Error("Blt failed from back to front.\n%s", HResultToString(hr));
    return hr >= 0;
}

// @00476ea0  (UNVERIFIED: not reached at runtime in a normal session, probably an options-menu entry)
void Scene::SetQuality(int level)
{
    static const int modes[5] = { 0x40, 0x80, 0x88, 0x89, 0x8a };   // D3DRMRENDER values; 0x89 = gouraud
    if (renderer == NULL || level < 0 || level > 4)
        return;
    renderer->device->SetQuality((D3DRMRENDERQUALITY)modes[level]);
}

// @00477040
void Scene::SetFrameTransform(IDirect3DRMFrame *frame, const float *m)
{
    HRESULT hr = frame->SetPosition(sceneChild, m[12], m[13], m[14]);
    if (hr != 0)
        renderer->Error("SetPosition failed.\n%s", HResultToString(hr));
    hr = frame->SetOrientation(sceneChild, m[8], m[9], m[10], m[4], m[5], m[6]);
    if (hr != 0)
        renderer->Error("SetOrientation failed.\n%s", HResultToString(hr));
}

// @004772d0
BOOL Scene::SetSceneChild(IDirect3DRMFrame *frame)
{
    if (!DeleteSceneChild())
        return FALSE;
    sceneChild = frame;
    HRESULT hr = renderer->sceneFrame->AddChild(frame);
    sceneChild->Release();   // drops the caller's reference; the scene frame now owns it
    if (hr != 0) {
        renderer->Error("AddChild failed.\n%s", HResultToString(hr));
        return FALSE;
    }
    return TRUE;
}

// @00477340
BOOL Scene::DeleteSceneChild()
{
    if (sceneChild) {
        HRESULT hr = renderer->sceneFrame->DeleteChild(sceneChild);
        if (hr != 0) {
            renderer->Error("DeleteChild failed.\n%s", HResultToString(hr));
            return FALSE;
        }
        sceneChild = NULL;
    }
    return TRUE;
}

// @00476f40
// New frame as a child of sceneChild. The parent keeps it alive, so our creation reference is dropped.
// (The original calls Release on an uninitialised pointer when CreateFrame fails; we return NULL instead.)
IDirect3DRMFrame *Scene::CreateSceneFrame()
{
    IDirect3DRMFrame *frame = NULL;
    HRESULT hr = renderer->d3drm->CreateFrame(sceneChild, &frame);
    if (hr != 0) {
        renderer->Error("CreateFrame failed.\n%s", HResultToString(hr));
        return NULL;
    }
    frame->Release();
    return frame;
}

// @00476f90
// Same with an explicit parent. Without a parent nobody else holds the frame, so the caller keeps our reference.
// The original reuses the "Render failed" text for the error message (copy and paste slip).
IDirect3DRMFrame *Scene::CreateFrameUnder(IDirect3DRMFrame *parent)
{
    IDirect3DRMFrame *frame = NULL;
    HRESULT hr = renderer->d3drm->CreateFrame(parent, &frame);
    if (hr != 0) {
        renderer->Error("Render failed.\n%s", HResultToString(hr));
        return NULL;
    }
    if (parent)
        frame->Release();
    return frame;
}

// @00477400
IDirect3DRMFrame *Scene::CreateDecalFrame(IDirect3DRMTexture *tex, float px, float py, float pz,
                                          int arg18, float sizeW, float sizeH, int arg24, float scaleX, float scaleY,
                                          int arg30, BOOL transparent, float colorR, float colorG, float colorB,
                                          IDirect3DRMFrame *parent)
{
    (void)arg18; (void)arg24; (void)arg30;
    LPD3DRMIMAGE image = tex->GetImage();
    IDirect3DRMTexture *decal = NULL;
    if (renderer->d3drm->CreateTexture(image, &decal) != 0 || decal == NULL)   // original ignores the result
        return NULL;
    LONG originX = (LONG)((double)image->width  * scaleX);
    LONG originY = (LONG)((double)image->height * scaleY);

    decal->SetDecalScale(1);
    decal->SetDecalTransparency(transparent);
    decal->SetDecalTransparentColor(D3DRMCreateColorRGB(colorR, colorG, colorB));
    decal->SetDecalOrigin(originX, originY);
    decal->SetDecalSize(sizeW, sizeH);

    IDirect3DRMFrame *frame = parent ? CreateFrameUnder(parent) : CreateSceneFrame();
    frame->AddVisual(decal);
    if (parent)
        frame->SetPosition(parent, px, py, pz);
    decal->Release();
    return frame;
}

// @00476ff0
BOOL DestroyFrame(IDirect3DRMFrame *frame)
{
    IDirect3DRMFrame *parent = NULL;
    frame->GetParent(&parent);
    if (parent) {
        parent->DeleteChild(frame);
        parent->Release();
    } else {
        frame->Release();
    }
    return TRUE;
}

// @00477120
// Fills m with the frame's transform relative to `ref`. The X axis is not stored by D3DRM, it is up x dir.
void GetFrameMatrix(IDirect3DRMFrame *frame, IDirect3DRMFrame *ref, float m[16])
{
    D3DVECTOR dir, up, pos;
    frame->GetOrientation(ref, &dir, &up);
    frame->GetPosition(ref, &pos);

    float rx = up.y * dir.z - up.z * dir.y;
    float ry = up.z * dir.x - up.x * dir.z;
    float rz = up.x * dir.y - up.y * dir.x;

    m[0] = rx;    m[1] = ry;    m[2] = rz;    m[3] = 0.0f;
    m[4] = up.x;  m[5] = up.y;  m[6] = up.z;  m[7] = 0.0f;
    m[8] = dir.x; m[9] = dir.y; m[10] = dir.z; m[11] = 0.0f;
    m[12] = pos.x; m[13] = pos.y; m[14] = pos.z; m[15] = 1.0f;
}
