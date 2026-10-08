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
#include "Float3.h"

// How a decal (a flat picture on a prop) looks: the 44 byte record the ini files describe with the keys <prefix>Colors, Origin,
// Size, UseTransparency and TransparentColor. The original passes it by value (11 dwords) to the functions below and to
// Scene::CreateDecalFrame, which uses size[0..1], origin[0..1] and the transparency fields. The unused values are kept because
// the ini reader fills up to three floats per key.
struct DecalSettings {
    int   colors;               // 0x00 "Colors", an integer (CreateDecalFrame ignores it)
    float size[3];              // 0x04 "Size": width and height of the decal (the third value is ignored)
    float origin[3];            // 0x10 "Origin": the decal origin as a fraction of the picture's width and height (the third value is ignored)
    int   useTransparency;      // 0x1c "UseTransparency"
    float transparentColor[3];  // 0x20 "TransparentColor": red, green, blue
};
static_assert(sizeof(DecalSettings) == 44, "DecalSettings layout");

struct NavMesh;

class World {
public:
    World();                                          // @004357b0 reads [Camera] FOV / Far / Near, sets the view, makes the frame timer
    ~World();                                         // @004358e0 frees the frame timer and the nav surface (not the lights)
    void CreateScene(IniReader *a, IniReader *b);     // @00435910 scene frame, palette, sky and nav surface of a level (reader b, or a when b is NULL)
    void DestroyScene();                              // @00435b30 releases the lights, the scene frame and the nav surface
    void LoadNavMesh(IniReader *reader);              // @004363a0 [Defaults] NavMesh: loads the nav surface file
    void SetupLights(IniReader *reader);              // @00436040 [Render] InitialLightPosition / Orientation / AmbientLightIntensity
    void RenderFrame();                               // @004362a0 clear, draw the scene and the panel, present; times each phase

    // Decal helpers. The world is the caller in the original, but none of them uses `this`.
    DecalSettings *ReadDecalSettings(DecalSettings *out, IniReader *reader, const char *section, const char *prefix,
                                     DecalSettings defaults);   // @00435c90 the [section] keys override `defaults`; returns out
    IDirect3DRMFrame *CreateDecal(IDirect3DRMTexture *texture, Float3 position, DecalSettings settings,
                                  IDirect3DRMFrame *parent);    // @00435bb0 forwards to Scene::CreateDecalFrame
    IDirect3DRMFrame *SetDecalSize(IDirect3DRMFrame *frame, float width, float height);   // @00435c70 forwards to SetFrameDecalSize

    int           unknown0;          // 0x00 never written or read by the code seen
    IDirect3DRMFrame *directionalLight;   // 0x04 frame of the sun
    IDirect3DRMFrame *ambientLight;       // 0x08 frame of the ambient light
    NavMesh      *navMesh;            // 0x0c the level's ground (NavMesh.h), NULL when the level has none
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
