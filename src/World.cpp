// World: see World.h. Reconstructed from SQZ.EXE 0x4357b0-0x4365a0.
#include <string.h>
#include <windows.h>
#include <mmsystem.h>
#include "World.h"
#include "Panel.h"
#include "Original.h"

// The frame timer the constructor makes (20 bytes). Only the first dword is used.
struct FrameTimer {
    DWORD last;         // 0x00 timeGetTime at the last lap
    int   unused[4];    // 0x04 cleared by the constructor, then unused[1] (offset 8) is set to 20; nothing seen reads them
};
static_assert(sizeof(FrameTimer) == 20, "FrameTimer layout");

static FrameTimer **const kFrameTimer = (FrameTimer **)0x4c7308;       // the timer object (a plain pointer in the original)
static FrameStats *const kFrameStats = (FrameStats *)0x4c730c;         // the counters RenderFrame keeps

// @004357b0
World::World()
{
    IniReader ini;
    directionalLight = NULL;
    ambientLight = NULL;
    navMesh = NULL;
    ini.SetSection("Camera");
    float fov = ini.GetFloat("FOV", 45.0f);
    float farPlane = ini.GetFloat("Far", 1000.0f);
    float nearPlane = ini.GetFloat("Near", 1.0f);
    GetScene()->SetViewParams(nearPlane, farPlane, fov);

    FrameTimer *timer = (FrameTimer *)Operator_New(sizeof(FrameTimer));
    // The original fills all 20 bytes in this order: three spare dwords cleared, the time cleared, then 20 stored at offset 8
    // (maybe a size field). Nothing seen reads the spare fields; they are written only so the object matches the original.
    if (timer) {
        timer->unused[3] = 0;
        timer->unused[2] = 0;
        timer->unused[0] = 0;
        timer->last = 0;
        timer->unused[1] = 20;
    }
    *kFrameTimer = timer;
    kFrameStats->totalClear = 0;
    kFrameStats->totalRender = 0;
    kFrameStats->totalPresent = 0;
    kFrameStats->frames = 0;
}

// @004358e0 The frame timer is freed but the global keeps pointing at it, and the lights are not touched (DestroyScene does that).
World::~World()
{
    Operator_Delete(*kFrameTimer);
    if (navMesh) {
        NavMeshDestroy(navMesh);
        Operator_Delete(navMesh);
    }
}

// @00435910
void World::CreateScene(IniReader *a, IniReader *b)
{
    Scene *scene = GetScene();
    scene->SetSceneChild(scene->CreateFrameUnder(NULL));

    IniReader *reader = b ? b : a;
    reader->SetSection("Defaults");

    const char *palette = reader->GetString("Palette");
    if (palette && strlen(palette) != 0) {
        unsigned char colors[1024];                     // 256 RGBA entries
        if (LoadPaletteFile(palette, colors)) {
            GetRenderer()->SetPaletteEntries(0, 256, colors);
            GameLog("Loaded palette from: \"%s\"\n", palette);
        } else {
            GameLog("Cannot load palette from: \"%s\"\n", palette);
        }
    } else {
        GameLog("Palette not specified, using existing palette.\n");
    }

    const char *warp = reader->GetString("WarpPrefix");
    if (strlen(warp) == 0) {
        GameLog("WARNING: World::CreateScene: no warp indicated in .ini file.\n");
    } else {
        GameLog("Loading warp background %s: ", warp);
        // A name without a path is looked up next to the ini file: switch there while it loads, then back.
        char savedDirectory[128];
        savedDirectory[0] = 0;
        if (!strchr(warp, '\\')) {
            GetCurrentDirectoryA(127, savedDirectory);
            String directory;
            SetCurrentDirectoryA(reader->GetDirectory(&directory)->c_str());
        }
        GameLog(scene->LoadWarp(warp) ? "OK.\n" : "failed!\n");
        if (strlen(savedDirectory) != 0)
            SetCurrentDirectoryA(savedDirectory);
    }

    LoadNavMesh(reader);
}

// @00435b30 The lights are not checked for NULL (the original never calls this without them).
void World::DestroyScene()
{
    Scene *scene = GetScene();
    DestroyFrame(ambientLight);
    ambientLight->Release();
    ambientLight = NULL;
    DestroyFrame(directionalLight);
    directionalLight->Release();
    directionalLight = NULL;
    ClearWorldObjects();
    scene->DeleteSceneChild();
    if (navMesh) {
        NavMeshDestroy(navMesh);
        Operator_Delete(navMesh);
    }
    navMesh = NULL;
}

// @004363a0
void World::LoadNavMesh(IniReader *reader)
{
    reader->SetSection("Defaults");
    const char *name = reader->GetString("NavMesh");
    if (strlen(name) == 0) {
        GameLog("No nav surface defined.\n");
        return;
    }
    void *memory = Operator_New(0x2030);
    navMesh = memory ? NavMeshConstruct(memory) : NULL;
    GameLog("Loading nav surface %s: ", name);

    // Like the sky: a bare file name is looked up next to the ini file.
    char savedDirectory[128];
    savedDirectory[0] = 0;
    if (!strchr(name, '\\')) {
        GetCurrentDirectoryA(127, savedDirectory);
        String directory;
        SetCurrentDirectoryA(reader->GetDirectory(&directory)->c_str());
    }
    if (!NavMeshLoad(navMesh, name)) {
        GameLog("failed!\n");
        if (navMesh) {
            NavMeshDestroy(navMesh);
            Operator_Delete(navMesh);
        }
        navMesh = NULL;
    } else {
        GameLog("OK.\n");
    }
    if (strlen(savedDirectory) != 0)
        SetCurrentDirectoryA(savedDirectory);
}

// b = up x direction, then c = b x direction: the "up" the sun's frame is given, perpendicular to its direction (@0041a350)
static void Cross(const float *a, const float *b, float *out)
{
    float x = a[1] * b[2] - a[2] * b[1];
    float y = a[2] * b[0] - a[0] * b[2];
    float z = a[0] * b[1] - a[1] * b[0];
    out[0] = x;
    out[1] = y;
    out[2] = z;
}

// @00436040
void World::SetupLights(IniReader *reader)
{
    reader->SetSection("Render");
    float color[3] = { 1.0f, 1.0f, 1.0f };
    float position[3];
    float direction[3];
    Scene *scene = GetScene();

    if (reader->GetFloats("InitialLightPosition", position) == 3 &&
        reader->GetFloats("InitialLightOrientation", direction) == 3) {
        if (directionalLight) {
            DestroyFrame(directionalLight);
            directionalLight->Release();
        }
        float up[3] = { 0.0f, 1.0f, 0.0f };
        float side[3];
        Cross(up, direction, side);
        Cross(side, direction, up);
        directionalLight = scene->CreateDirectionalLight(color[0], color[1], color[2],
                                                         position[0], position[1], position[2],
                                                         direction[0], direction[1], direction[2],
                                                         up[0], up[1], up[2]);
    }

    if (reader->GetFloats("AmbientLightIntensity", color) == 3) {
        if (ambientLight) {
            DestroyFrame(ambientLight);
            ambientLight->Release();
        }
        ambientLight = scene->CreateAmbientLight(color[0], color[1], color[2]);
    }
}

// @004362a0 Draws one frame and adds up how long each phase took. The timer is restarted after the first two laps only.
void World::RenderFrame()
{
    Scene *scene = GetScene();
    FrameTimer *timer;

    kFrameStats->frames++;
    timer = *kFrameTimer;
    timer->last = timeGetTime();
    scene->BeginFrame();

    timer = *kFrameTimer;
    int lap = timeGetTime() - timer->last;
    kFrameStats->lastClear = lap;
    kFrameStats->totalClear += lap;
    timer->last = timeGetTime();
    scene->Render();
    Panel *panel = (Panel *)GetPanel();
    if (panel)
        panel->Update(0);

    timer = *kFrameTimer;
    lap = timeGetTime() - timer->last;
    kFrameStats->lastRender = lap;
    kFrameStats->totalRender += lap;
    timer->last = timeGetTime();
    scene->Present();

    timer = *kFrameTimer;
    lap = timeGetTime() - timer->last;
    kFrameStats->lastPresent = lap;
    kFrameStats->totalPresent += lap;
}

// key = prefix + suffix (the original builds it with strcpy and strcat into a stack buffer)
static void BuildKey(char *key, const char *prefix, const char *suffix)
{
    strcpy(key, prefix);
    strcat(key, suffix);
}

// @00435c90 Each key only overrides the default when the ini file has it.
DecalSettings *World::ReadDecalSettings(DecalSettings *out, IniReader *reader, const char *section, const char *prefix,
                                        DecalSettings defaults)
{
    char key[128];
    reader->SetSection(section);

    BuildKey(key, prefix, "Colors");
    if (strlen(reader->GetString(key)) != 0)
        defaults.colors = reader->GetInt(key, 0);

    BuildKey(key, prefix, "Origin");
    if (strlen(reader->GetString(key)) != 0)
        reader->GetFloats(key, defaults.origin);

    BuildKey(key, prefix, "Size");
    if (strlen(reader->GetString(key)) != 0)
        reader->GetFloats(key, defaults.size);

    BuildKey(key, prefix, "UseTransparency");
    if (strlen(reader->GetString(key)) != 0)
        defaults.useTransparency = reader->GetInt(key, 0);

    BuildKey(key, prefix, "TransparentColor");
    if (strlen(reader->GetString(key)) != 0)
        reader->GetFloats(key, defaults.transparentColor);

    *out = defaults;
    return out;
}

// CreateDecalFrame takes the record's unused values as plain dwords (its colors, size[2] and origin[2] arguments)
static int Bits(float f)
{
    int bits;
    memcpy(&bits, &f, sizeof bits);
    return bits;
}

// @00435bb0
IDirect3DRMFrame *World::CreateDecal(IDirect3DRMTexture *texture, Float3 position, DecalSettings settings,
                                     IDirect3DRMFrame *parent)
{
    return GetScene()->CreateDecalFrame(texture, position.x, position.y, position.z,
                                        settings.colors, settings.size[0], settings.size[1], Bits(settings.size[2]),
                                        settings.origin[0], settings.origin[1], Bits(settings.origin[2]),
                                        settings.useTransparency,
                                        settings.transparentColor[0], settings.transparentColor[1], settings.transparentColor[2],
                                        parent);
}

// @00435c70
IDirect3DRMFrame *World::SetDecalSize(IDirect3DRMFrame *frame, float width, float height)
{
    return SetFrameDecalSize(frame, width, height);
}
