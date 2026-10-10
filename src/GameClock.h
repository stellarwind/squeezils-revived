// GameClock: the game's timer (the object in the global at 0x4badc0, 0x30 bytes, no vtable; code 0x41eb90-0x41ed70). It makes the
// simulation run at a steady rate: every simulation step stands for a fixed number of game milliseconds (SimClockRateMS), and the
// clock keeps count of how much real time the steps and the drawing used. The step lengths it measures are cut off at
// MaxSimTimeMS and MaxRenderTimeMS (so a long stall does not become one huge step) and scaled by TimeMultiplier.
// All three are read from the [Defaults] section of sqz.ini by the game start-up (0x420170): 50, 200, 1000 and 0.
//
// Two stopwatches: the "sim" one runs around a simulation step (BeginSim / EndSim, from RunFrame); the "render" one is restarted
// by ResetRender and read by EndRender (from ResetClock, which draws one frame after a level change).
#pragma once

struct GameClock {
    unsigned start;          // 0x00 timeGetTime at construction (never read)
    unsigned maxSimMs;       // 0x04 MaxSimTimeMS: the longest sim step counted
    unsigned maxRenderMs;    // 0x08 MaxRenderTimeMS
    int      simRateMs;      // 0x0c SimClockRateMS: the game time one simulation step stands for
    unsigned renderStart;    // 0x10
    unsigned renderEnd;      // 0x14 renderStart plus the scaled, limited time since
    unsigned simStart;       // 0x18
    unsigned simEnd;         // 0x1c the same for the sim stopwatch
    int      gameTime;       // 0x20 the sum of the fixed steps taken so far
    int      usedTime;       // 0x24 the sum of the measured step and render times
    int      balance;        // 0x28 game time taken minus real time used: above 0 when there is time to spare
    float    multiplier;     // 0x2c TimeMultiplier (zero or less: no scaling)

    GameClock(int simRateMs, unsigned maxSimMs, unsigned maxRenderMs, float multiplier);   // @0041eb90 (thiscall, ret 0x10) also timeBeginPeriod(1)
    ~GameClock();                                       // @0041ebe0 timeEndPeriod(1)

    void ResetRender();                                 // @0041ec20 restarts the render stopwatch
    void EndRender();                                   // @0041ec30 counts the time since as a render step
    void BeginSim();                                    // @0041eca0 restarts the sim stopwatch
    void EndSim();                                      // @0041ecb0 counts the time since as a sim step, and one fixed step of game time

    int  Ahead() { return balance > 0; }                // @0041ed40
    int  RenderStep() { return renderEnd - renderStart; }   // @0041ed50
    int  SimStep() { return simEnd - simStart; }        // @0041ed60
    // @0041ebf0 / @0041ec00 / @0041ec10 are the plain reads of usedTime, simRateMs and gameTime (too short to hook)

private:
    unsigned Scaled(unsigned from, unsigned limit);     // the time since `from`, scaled, then limited (the common part of the two End functions)
};
static_assert(sizeof(GameClock) == 0x30, "GameClock layout");

// the global at 0x4badc0
inline GameClock *&TheClock() { return *(GameClock **)0x4badc0; }
