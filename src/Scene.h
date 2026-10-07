// Scene: per-frame draw path on top of Renderer (D3DRM frames, z-buffer clear, optional warp backdrop, Render, Present).
// Reconstructed from SQZ.EXE 0x476720-0x477400. "Scene" and "Warp" are our names (no RTTI). The single game
// instance is the global at 0x4c75e0. Field offsets are the original ones.
#pragma once
#include "Renderer.h"

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
    IDirect3DRMFrame *CreateSceneFrame();                 // 0x476f40 new frame under sceneChild (parent owns it)
    IDirect3DRMFrame *CreateFrameUnder(IDirect3DRMFrame *parent);   // 0x476f90 parent == NULL: caller owns the frame
    // 0x477400 builds a decal sprite: clones the texture image, applies decal settings, wraps it in a new frame.
    // The original takes 16 stack dwords (two of them are by-value helper objects at 0x0c and 0x18; their
    // meaning beyond the fields used here is not known, unused slots are named after their stack offset).
    IDirect3DRMFrame *CreateDecalFrame(IDirect3DRMTexture *tex, float px, float py, float pz,
                                       int arg18, float sizeW, float sizeH, int arg24, float scaleX, float scaleY,
                                       int arg30, BOOL transparent, float colorR, float colorG, float colorB,
                                       IDirect3DRMFrame *parent);
    IDirect3DRMFrame *GetSceneChild() const { return sceneChild; }              // 0x477290
    IDirect3DRMFrame *GetCameraFrame() const { return renderer->cameraFrame; }  // 0x477390

    Renderer             *renderer;     // 0x00
    LPDIRECTDRAWSURFACE   warpSurface;  // 0x04 offscreen, half the render size
    IDirect3DRMFrame     *lightsFrame;  // 0x08
    IDirect3DRMFrame     *sceneChild;   // 0x0c
    void                 *warp;         // 0x10 optional Warp object (0x1e4 bytes), not reconstructed yet
    int                   warpWidth;    // 0x14
    int                   warpHeight;   // 0x18
};

// Free helpers that work on any D3DRM frame
BOOL DestroyFrame(IDirect3DRMFrame *frame);                                   // 0x476ff0 detach from parent (or release) and drop our reference
void GetFrameMatrix(IDirect3DRMFrame *frame, IDirect3DRMFrame *ref, float m[16]);   // 0x477120 inverse of SetFrameTransform: rows right, up, dir, position
