// GameClock.cpp: reconstructed from SQZ.EXE 0x41eb90-0x41ed70, and the two functions that drive it every frame (RunFrame
// @004206e0, ResetClock @00420760). See GameClock.h.
#include <windows.h>
#include <string.h>
#include "GameClock.h"
#include "Original.h"
#include "PropList.h"
#include "World.h"
#include "VirtualCall.h"

GameClock::GameClock(int simRate, unsigned maxSim, unsigned maxRender, float scale)
{
    timeBeginPeriod(1);
    unsigned now = timeGetTime();
    renderEnd = renderStart = simEnd = simStart = start = now;
    gameTime = 0;
    usedTime = 0;
    balance = 0;
    simRateMs = simRate;
    maxSimMs = maxSim;
    maxRenderMs = maxRender;
    multiplier = scale;
}

GameClock::~GameClock()
{
    timeEndPeriod(1);
}

void GameClock::ResetRender() { renderStart = timeGetTime(); }

void GameClock::BeginSim() { simStart = timeGetTime(); }

// The time since `from` (unsigned, as a 64 bit integer in the x87), times the multiplier when that is positive (the original
// tests the float's bits as an integer), truncated; the result is added back onto `from` and the step limited.
unsigned GameClock::Scaled(unsigned from, unsigned limit)
{
    unsigned elapsed = timeGetTime() - from;
    int bits;
    memcpy(&bits, &multiplier, sizeof bits);
    unsigned to = from + elapsed;
    if (bits > 0)
        to = from + (unsigned)(int)((long double)elapsed * multiplier);
    return to - from > limit ? from + limit : to;
}

void GameClock::EndRender()
{
    renderEnd = Scaled(renderStart, maxRenderMs);
    usedTime += RenderStep();
    balance -= RenderStep();
}

void GameClock::EndSim()
{
    simEnd = Scaled(simStart, maxSimMs);
    usedTime += SimStep();
    gameTime += simRateMs;
    balance += simRateMs - SimStep();
}

// ---- the callers ----

unsigned FrameMilliseconds() { return TheClock()->simRateMs; }    // @004206d0 then @0041ec00
int      ClockValueA()       { return TheClock()->gameTime; }     // @004206d0 then @0041ec10
int      ClockValueB()       { return TheClock()->usedTime; }     // @004206d0 then @0041ebf0

// @004206e0 One game step: the clock's sim stopwatch runs around it. The elapsed game time is the clock's fixed step, in seconds.
void RunFrame()
{
    GameClock *clock = TheClock();
    float seconds = (float)((long double)(unsigned)clock->simRateMs * 0.001f);   // (the original converts the unsigned count in the x87)
    clock->BeginSim();
    AudioUpdate();
    ScriptUpdateTimers(seconds);
    UpdateProps(seconds);
    void *simulation = GetCurrentSimulation();
    if (simulation) {
        CallVirtual<void>(simulation, 8);                 // CollideCreatures
        CallVirtual<void>(GetCurrentSimulation(), 3);     // UpdateSouls
    }
    TheClock()->EndSim();
}

// @00420760 Starts over after a level change: the render stopwatch, the script timers and prop levels, the simulation's actors
// (slot 4) and one frame drawn, then the render time is counted.
void ResetClock()
{
    TheClock()->ResetRender();
    ScriptResetTimers();
    UpdateLevels();
    void *simulation = GetCurrentSimulation();
    if (simulation)
        CallVirtual<void>(simulation, 4);                 // RunActors
    GetWorld()->RenderFrame();
    TheClock()->EndRender();
}
