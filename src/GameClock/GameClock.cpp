// GameClock (SQZ.EXE 0x41eb90-0x41ed70), functions in address order.
#include <windows.h>
#include <mmsystem.h>
#include "GameClock/GameClock.h"

GameClock::GameClock(int simRate, unsigned maxSim, unsigned maxRender, float scale)
{
    timeBeginPeriod(1);
    unsigned now = timeGetTime();
    start = renderStart = renderEnd = simStart = simEnd = now;
    balance = 0;
    gameTime = 0;
    usedTime = 0;
    simRateMs = simRate;
    maxSimMs = maxSim;
    maxRenderMs = maxRender;
    multiplier = scale;
}

GameClock::~GameClock()
{
    timeEndPeriod(1);
}

int GameClock::UsedTime()
{
    return usedTime;
}

int GameClock::SimRate()
{
    return simRateMs;
}

int GameClock::GameTime()
{
    return gameTime;
}

inline unsigned GameClock::Scaled(unsigned from, unsigned limit)
{
    unsigned elapsed = timeGetTime() - from;
    unsigned to = elapsed;
    if (*(int *)&multiplier > 0)
        to = (int)((double)elapsed * multiplier);
    to += from;
    if (to - from > limit)
        return from + limit;
    return to;
}

void GameClock::ResetRender()
{
    renderStart = timeGetTime();
}

void GameClock::EndRender()
{
    unsigned now = timeGetTime();
    unsigned from = renderStart;
    unsigned elapsed = now - from;
    unsigned to = elapsed;
    if (*(int *)&multiplier > 0)
        to = (int)((double)elapsed * multiplier);
    to += from;
    if (to - from > maxRenderMs) {
        from += maxRenderMs;
        renderEnd = from;
    } else
        renderEnd = to;
    usedTime += RenderStep();
    balance = balance - RenderStep();
}

void GameClock::BeginSim()
{
    simStart = timeGetTime();
}

void GameClock::EndSim()
{
    unsigned now = timeGetTime();
    unsigned from = simStart;
    unsigned elapsed = now - from;
    int step = elapsed;
    if (*(int *)&multiplier > 0)
        step = (int)((double)elapsed * multiplier);
    unsigned to = from + step;
    if (to - from > maxSimMs)
        simEnd = from + maxSimMs;
    else
        simEnd = to;
    usedTime += SimStep();
    gameTime += SimRate();
    balance += SimRate() - SimStep();
    SimStep();
    SimRate();
}

int GameClock::Ahead()
{
    return Balance() > 0;
}

int GameClock::RenderStep()
{
    return renderEnd - renderStart;
}

int GameClock::SimStep()
{
    return simEnd - simStart;
}

int GameClock::Balance()
{
    return balance;
}
