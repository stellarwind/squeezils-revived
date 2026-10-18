// World::World, SQZ.EXE 0x4357b0.
#include "World/World.h"
#include "World/WorldRuntime.h"
#include "IniReader/IniReader.h"

extern unsigned long gWorldTotalClear, gWorldTotalRender, gWorldTotalPresent;
extern unsigned long gWorldFrames;

World::World()
{
    IniReader ini;
    navMesh = 0;
    ambientLight = 0;
    directionalLight = 0;
    ini.SetSection("Camera");
    gWorldScene->SetViewParams(ini.GetFloat("Near", 1.0f),
        ini.GetFloat("Far", 1000.0f), ini.GetFloat("FOV", 90.0f));
    gFrameTimer = new WorldFrameTimer;
    gWorldTotalClear = 0;
    gWorldTotalRender = 0;
    gWorldTotalPresent = 0;
    gWorldFrames = 0;
}
