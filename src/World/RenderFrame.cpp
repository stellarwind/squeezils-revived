// World::RenderFrame, SQZ.EXE 0x4362a0.
#include <windows.h>
#include <mmsystem.h>
#include "World/World.h"
#include "World/WorldRuntime.h"

inline void WorldFrameTimer::Start() { last = timeGetTime(); }
inline unsigned long WorldFrameTimer::Lap() { return timeGetTime() - last; }
struct WorldPanel { int Update(int force); }; // 0x42db10
WorldPanel *GetPanel(); // 0x4206c0
extern unsigned long gWorldLastClear, gWorldLastRender, gWorldLastPresent;
extern unsigned long gWorldTotalClear, gWorldTotalRender, gWorldTotalPresent;
extern unsigned long gWorldFrames;

void World::RenderFrame()
{
    ++gWorldFrames;
    gFrameTimer->Start();
    gWorldScene->BeginFrame();
    unsigned long lap = gFrameTimer->Lap();
    gWorldLastClear = lap;
    gWorldTotalClear += lap;
    gFrameTimer->Start();
    gWorldScene->Render();
    if (GetPanel())
        GetPanel()->Update(0);
    lap = gFrameTimer->Lap();
    gWorldLastRender = lap;
    gWorldTotalRender += lap;
    gFrameTimer->Start();
    gWorldScene->Present();
    lap = gFrameTimer->Lap();
    gWorldLastPresent = lap;
    gWorldTotalPresent += lap;
}
