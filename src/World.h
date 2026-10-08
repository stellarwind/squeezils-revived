// World: the level's scene set-up. Reconstructed from SQZ.EXE 0x4357b0-0x4365a0. "World" is our name; the original's own log
// message says "World::CreateScene" (the string at 0x4bc460). One instance, a 16 byte object held in the global at 0x4bada8
// (made at the game's start, 0x4202b3, deleted at 0x42060f).
//
// It owns the level's two lights and its navigation surface, reads the camera defaults, loads the palette and the sky when a
// level's ini file says so, and draws the frame (with a millisecond count per phase). The props, the loaders that call it
// (0x4170c8, 0x438c8d) and the rest of the level code are still original.
#pragma once
#include "IniReader.h"
#include "Scene.h"

class World {
public:
    World();                                          // @004357b0 reads [Camera] FOV / Far / Near, sets the view, makes the frame timer
    ~World();                                         // @004358e0 frees the frame timer and the nav surface (not the lights)
    void CreateScene(IniReader *a, IniReader *b);     // @00435910 scene frame, palette, sky and nav surface of a level (reader b, or a when b is NULL)
    void DestroyScene();                              // @00435b30 releases the lights, the scene frame and the nav surface
    void LoadNavMesh(IniReader *reader);              // @004363a0 [Defaults] NavMesh: loads the nav surface file
    void SetupLights(IniReader *reader);              // @00436040 [Render] InitialLightPosition / Orientation / AmbientLightIntensity
    void RenderFrame();                               // @004362a0 clear, draw the scene and the panel, present; times each phase

    int           unknown0;           // 0x00 never written or read by the code seen
    IDirect3DRMFrame *directionalLight;   // 0x04 frame of the sun
    IDirect3DRMFrame *ambientLight;       // 0x08 frame of the ambient light
    void         *navMesh;            // 0x0c the nav surface object (0x2030 bytes, original class), NULL when the level has none
};
static_assert(sizeof(World) == 16, "World layout");
static_assert(offsetof(World, directionalLight) == 4 && offsetof(World, navMesh) == 12, "World layout");

// The millisecond counters RenderFrame keeps (globals at 0x4c730c..0x4c7324; nothing else reads them in the code seen).
struct FrameStats {
    int lastClear;      // 0x4c730c milliseconds the last BeginFrame took
    int lastRender;     // 0x4c7310 Render and the panel update
    int lastPresent;    // 0x4c7314 Present
    int totalClear;     // 0x4c7318 the sum over all frames
    int totalRender;    // 0x4c731c
    int totalPresent;   // 0x4c7320
    int frames;         // 0x4c7324 frames drawn
};
