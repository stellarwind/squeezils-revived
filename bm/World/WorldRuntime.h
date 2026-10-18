// Shared external interfaces used by the World translation units.
#pragma once
struct IDirect3DRMFrame;
struct IDirect3DRMTexture;
struct WorldVector;
struct WorldDecalSettings;
struct WorldScene {
    void DestroyFrame(IDirect3DRMFrame *frame); // 0x476ff0
    void DeleteSceneChild(); // 0x477340
    int BeginFrame(); // 0x476af0
    int Render();     // 0x476cd0
    int Present();    // 0x476d20
    IDirect3DRMFrame *CreateFrameUnder(IDirect3DRMFrame *parent); // 0x476f90
    void SetSceneChild(IDirect3DRMFrame *frame); // 0x4772d0
    int LoadWarp(const char *prefix); // 0x4769a0
    IDirect3DRMFrame *CreateDirectionalLight(WorldVector color, WorldVector position,
        WorldVector direction, WorldVector up); // 0x4775c0
    IDirect3DRMFrame *CreateAmbientLight(WorldVector color); // 0x477700
    void SetViewParams(float nearPlane, float farPlane, float fov); // 0x476e40
    IDirect3DRMFrame *CreateDecalFrame(IDirect3DRMTexture *texture, WorldVector position,
        WorldDecalSettings settings, IDirect3DRMFrame *parent); // 0x477400
};
struct WorldFrameTimer {
    unsigned long last;
    unsigned long spare0, size, spare1, spare2;
    WorldFrameTimer() { spare2 = 0; spare1 = 0; spare0 = 0; last = 0; size = 20; }
    void Start();
    unsigned long Lap();
};
extern WorldScene *gWorldScene;      // 0x4c75e0
extern WorldFrameTimer *gFrameTimer; // 0x4c7308
