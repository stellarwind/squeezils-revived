// Scene (the per-frame draw path on top of Renderer, the single instance is the global at 0x4c75e0) for the byte-match build.
#pragma once
#include "Renderer2/Renderer2.h"

struct WarpSky {                       // the sky object (0x1e4 bytes); only the fields the Scene touches
    int height;                        // +0
    int width;                         // +4
    float fov;                         // +8
    float fovScaled;                   // +0xc
    WarpSky();                                     // 0x478c80
    ~WarpSky();                                    // 0x478cb0
    int  Init(int h, int w, float fov, char flag);  // 0x478ce0
    int  Load(const char *prefix, unsigned char *palette, const int *fmt);   // 0x479090
    char pad[0x1e4 - 0x10];
    void Update(const float *m);                   // 0x478e10
    void Resize(int width, int height);            // 0x478dc0
    void SetOutput(void *bits);                    // 0x478da0
};

extern void SkySetFov(float a, float b);           // 0x48c680

struct RendererV : public Renderer {
    void SetViewParams(float nearPlane, float farPlane, float fov);   // 0x474f40
};

struct WorldVector;
struct WorldDecalSettings;

class Scene {
public:
    Scene();                                                              // 0x476720
    ~Scene();                                                             // 0x476740
    int  Render();                                                        // 0x476cd0
    int  EnsureWarpSurface();                                             // 0x4767a0
    int  Init(Renderer *r);                                               // 0x4768f0
    int  Present();                                                       // 0x476d20
    int  BeginFrame();                                                    // 0x476af0
    int  LoadWarp(const char *name);                                      // 0x4769a0
    void SetCameraTransform(const float *m);                              // 0x476e10
    void SetViewParams(float nearPlane, float farPlane, float fovDegrees);   // 0x476e40
    void SetQuality(int level);                                           // 0x476ea0
    LPDIRECT3DRMFRAME CreateSceneFrame();                                 // 0x476f40
    LPDIRECT3DRMFRAME CreateFrameUnder(LPDIRECT3DRMFRAME parent);         // 0x476f90
    void SetFrameTransform(LPDIRECT3DRMFRAME frame, const float *m);      // 0x477040
    int  SetSceneChild(LPDIRECT3DRMFRAME frame);                          // 0x4772d0
    int  DeleteSceneChild();                                              // 0x477340
    LPDIRECT3DRMFRAME GetSceneChild();                                    // 0x477290
    LPDIRECT3DRMFRAME GetCameraFrame();                                   // 0x477390
    LPDIRECT3DRMFRAME CreateDecalFrame(LPDIRECT3DRMTEXTURE texture, WorldVector position,
        WorldDecalSettings settings, LPDIRECT3DRMFRAME parent);            // 0x477400

    Renderer *renderer;                // +0
    LPDIRECTDRAWSURFACE warpSurface;   // +4
    LPDIRECT3DRMFRAME lightsFrame;     // +8
    LPDIRECT3DRMFRAME sceneChild;      // +0xc
    WarpSky *warp;                     // +0x10
    int warpWidth;                     // +0x14
    int warpHeight;                    // +0x18
};
extern void SkyDraw();      // 0x48c710
