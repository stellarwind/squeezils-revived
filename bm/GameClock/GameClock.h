// GameClock: the game's timer (0x30 bytes, no vtable; SQZ.EXE 0x41eb90-0x41ed70), VC++ 4.1 dialect. Behaviour: Repo/src/GameClock.h.
#pragma once

struct GameClock {
    unsigned start;          // 0x00
    unsigned maxSimMs;       // 0x04
    unsigned maxRenderMs;    // 0x08
    int simRateMs;           // 0x0c
    unsigned renderStart;    // 0x10
    unsigned renderEnd;      // 0x14
    unsigned simStart;       // 0x18
    unsigned simEnd;         // 0x1c
    int gameTime;            // 0x20
    int usedTime;            // 0x24
    int balance;             // 0x28
    float multiplier;        // 0x2c

    GameClock(int simRate, unsigned maxSim, unsigned maxRender, float scale);   // 0x41eb90
    ~GameClock();                                       // 0x41ebe0
    int UsedTime();                                     // 0x41ebf0
    int SimRate();                                      // 0x41ec00
    int GameTime();                                     // 0x41ec10
    void ResetRender();                                 // 0x41ec20
    void EndRender();                                   // 0x41ec30
    void BeginSim();                                    // 0x41eca0
    void EndSim();                                      // 0x41ecb0
    int Ahead();                                        // 0x41ed40
    int RenderStep();                                   // 0x41ed50
    int SimStep();                                      // 0x41ed60
    int Balance();                                      // 0x41ed70
    unsigned Scaled(unsigned from, unsigned limit);     // the time since from, scaled, then limited
};
