// World::CreateScene, SQZ.EXE 0x435910.
#include <windows.h>
#include <string.h>
#include "World/World.h"
#include "World/WorldRuntime.h"
#include "IniReader/IniReader.h"

struct WorldRenderer {
    int SetPaletteEntries(int first, int count, const unsigned char *colors); // 0x474c00
};
extern WorldRenderer *gWorldRenderer; // 0x4c75dc
int LoadPaletteFile(const char *name, unsigned char *colors); // 0x474ca0
int DebugLogAll(const char *format, ...); // 0x419290

void World::CreateScene(IniReader *a, IniReader *b)
{
    IDirect3DRMFrame *frame = gWorldScene->CreateFrameUnder(0);
    gWorldScene->SetSceneChild(frame);
    IniReader *reader = b ? b : a;
    reader->SetSection("Defaults");
    const char *palette = reader->GetString("Palette");
    if (palette && strlen(palette)) {
        unsigned char colors[1024];
        if (LoadPaletteFile(palette, colors)) {
            gWorldRenderer->SetPaletteEntries(0, 256, colors);
            DebugLogAll("Loaded palette from: \"%s\"\n", palette);
        } else {
            DebugLogAll("Cannot load palette from: \"%s\"\n", palette);
        }
    } else {
        DebugLogAll("Palette not specified, using existing palette.\n");
    }
    const char *warp = reader->GetString("WarpPrefix");
    if (!strlen(warp)) {
        DebugLogAll("WARNING: World::CreateScene: no warp indicated in .ini file.\n");
    } else {
        DebugLogAll("Loading warp background %s: ", warp);
        char savedDirectory[128];
        strcpy(savedDirectory, kEmptyString);
        if (!strchr(warp, '\\')) {
            GetCurrentDirectoryA(127, savedDirectory);
            SetCurrentDirectoryA(reader->GetDirectory().c_str());
        }
        if (gWorldScene->LoadWarp(warp))
            DebugLogAll("OK.\n");
        else
            DebugLogAll("failed!\n");
        if (strlen(savedDirectory))
            SetCurrentDirectoryA(savedDirectory);
    }
    LoadNavMesh(reader);
}
