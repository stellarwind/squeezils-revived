// Scene: per-frame draw path on top of Renderer (D3DRM frames, z-buffer clear, optional warp backdrop, Render, Present).
// Reconstructed from SQZ.EXE 0x476720-0x477400. "Scene" and "Warp" are our names (no RTTI). The single game
// instance is the global at 0x4c75e0. Field offsets are the original ones.
#pragma once
#include "Renderer.h"

class Warp;

class Scene {
public:
    Scene();                                              // 0x476720
    ~Scene();                                             // 0x476740

    BOOL  EnsureWarpSurface();                            // 0x4767a0
    BOOL  Init(Renderer *r);                              // 0x4768f0 (thiscall, 1 stack arg)
    BOOL  BeginFrame();                                   // 0x476af0 clears z-buffer, then back buffer or warp backdrop
    BOOL  Render();                                       // 0x476cd0 Viewport::Render(scene)
    BOOL  Present();                                      // 0x476d20 blits back buffer to the primary surface
    void  SetQuality(int level);                          // 0x476ea0 0..4 -> D3DRMRENDER mode
    void  SetFrameTransform(IDirect3DRMFrame *frame, const float *m);   // 0x477040 (position m[12..14], dir m[8..10], up m[4..6])
    BOOL  SetSceneChild(IDirect3DRMFrame *frame);         // 0x4772d0 attaches `frame` under the renderer's scene frame
    BOOL  DeleteSceneChild();                             // 0x477340
    BOOL  LoadWarp(const char *name);                     // 0x4769a0 loads the sky named by the world's WarpPrefix
    void  SetCameraTransform(const float *m);             // 0x476e10 moves the camera frame and draws the sky for the same view
    void  SetViewParams(float nearPlane, float farPlane, float fovDegrees);   // 0x476e40
    IDirect3DRMFrame *CreateSceneFrame();                 // 0x476f40 new frame under sceneChild (parent owns it)
    IDirect3DRMFrame *CreateFrameUnder(IDirect3DRMFrame *parent);   // 0x476f90 parent == NULL: caller owns the frame
    // 0x477400 builds a decal sprite: clones the texture image, applies decal settings, wraps it in a new frame.
    // The original takes 16 stack dwords (two of them are by-value helper objects at 0x0c and 0x18; their
    // meaning beyond the fields used here is not known, unused slots are named after their stack offset).
    IDirect3DRMFrame *CreateDecalFrame(IDirect3DRMTexture *tex, float px, float py, float pz,
                                       int arg18, float sizeW, float sizeH, int arg24, float scaleX, float scaleY,
                                       int arg30, BOOL transparent, float colorR, float colorG, float colorB,
                                       IDirect3DRMFrame *parent);
    // Lights. Each makes a new frame under lightsFrame, creates the light, attaches it and returns the frame (caller keeps
    // the creation reference). D3DRMLIGHT_AMBIENT = 0, D3DRMLIGHT_DIRECTIONAL = 3.
    IDirect3DRMFrame *CreateAmbientLight(float r, float g, float b);             // 0x477700 (thiscall, 3 stack args)
    IDirect3DRMFrame *CreateDirectionalLight(float r, float g, float b,          // 0x4775c0 (thiscall, 12 stack args)
                                             float px, float py, float pz,       // position relative to the scene frame
                                             float dx, float dy, float dz,       // direction
                                             float ux, float uy, float uz);      // up
    // 0x4777b0 (thiscall, 15 floats and 3 doubles on the stack): a flat colored quad in a new frame. Four corners, one normal,
    // color from r, g, b in 0..1. Returns the frame, or NULL if any D3DRM call fails.
    IDirect3DRMFrame *CreateColoredQuad(float x0, float y0, float z0, float x1, float y1, float z1,
                                        float x2, float y2, float z2, float x3, float y3, float z3,
                                        float nx, float ny, float nz, double r, double g, double b);
    IDirect3DRMFrame *GetSceneChild() const { return sceneChild; }              // 0x477290
    IDirect3DRMFrame *GetCameraFrame() const { return renderer->cameraFrame; }  // 0x477390

    Renderer             *renderer;     // 0x00
    LPDIRECTDRAWSURFACE   warpSurface;  // 0x04 offscreen, half the render size
    IDirect3DRMFrame     *lightsFrame;  // 0x08
    IDirect3DRMFrame     *sceneChild;   // 0x0c
    Warp                 *warp;         // 0x10 the sky (0x1e4 bytes), created by LoadWarp, NULL when the world has none
    int                   warpWidth;    // 0x14
    int                   warpHeight;   // 0x18
};

// Free helpers that work on any D3DRM frame
BOOL DestroyFrame(IDirect3DRMFrame *frame);                                   // 0x476ff0 detach from parent (or release) and drop our reference
void GetFrameMatrix(IDirect3DRMFrame *frame, IDirect3DRMFrame *ref, float m[16]);   // 0x477120 inverse of SetFrameTransform: rows right, up, dir, position
IDirect3DRMFrame *GetFrameParent(IDirect3DRMFrame *frame);                    // 0x4772a0 parent frame, no reference kept
HRESULT AddVisualToFrame(IDirect3DRMFrame *frame, IDirect3DRMVisual *visual);        // 0x4773a0
HRESULT AddChildToFrame(IDirect3DRMFrame *frame, IDirect3DRMFrame *child);            // 0x4773c0
HRESULT DeleteChildFromFrame(IDirect3DRMFrame *frame, IDirect3DRMFrame *child);       // 0x4773e0
void ApplyFrameTransform(IDirect3DRMFrame *frame, IDirect3DRMFrame *ref, const float *m);   // 0x4770d0 like Scene::SetFrameTransform, relative to ref, no error check
IDirect3DRMFrame *SetFrameDecalSize(IDirect3DRMFrame *frame, float width, float height);    // 0x477540 for a frame holding exactly one decal texture; returns the frame, or NULL
