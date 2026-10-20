// World layout from SQZ.EXE; 16 bytes on the original Win32 compiler.
#pragma once

struct NavMesh;
class IniReader;
struct IDirect3DRMFrame;
struct IDirect3DRMTexture;
struct WorldVector;
struct WorldDecalSettings;
struct World {
    int unknown0;
    void *directionalLight;
    void *ambientLight;
    NavMesh *navMesh;

    World();  // 0x4357b0
    ~World(); // 0x4358e0
    void DestroyScene(); // 0x435b30
    NavMesh *SurfacesOf(int face); // 0x4365a0
    void RenderFrame(); // 0x4362a0
    void LoadNavMesh(IniReader *reader); // 0x4363a0
    void CreateScene(IniReader *a, IniReader *b); // 0x435910
    void SetupLights(IniReader *reader); // 0x436040
    IDirect3DRMFrame *CreateDecal(IDirect3DRMTexture *texture, WorldVector position,
        WorldDecalSettings settings, IDirect3DRMFrame *parent); // 0x435bb0
};
