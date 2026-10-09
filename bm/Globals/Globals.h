// Globals: the game's global-object getters/setters (one load/store of a global pointer each).
#pragma once
#include "compat41.h"

void *GetLand();                              // 0x420680 (global 0x4badb8)
void *GetWorld();                             // 0x4206a0 (global 0x4bada8)
void *GetCurrentSimulation();                 // 0x420690 (global 0x4badbc)
void *GetApplication();                       // 0x4206b0 (global 0x4badac)
void *GetPanel();                             // 0x4206c0 (global 0x4badb0)
void *GetGameClock();                         // 0x4206d0 (global 0x4badc0)
void  SetCurrentSimulation(void *simulation); // 0x420670
void  SqgSetErrorCallback(void *fn);          // 0x4736f0 (global 0x4c0f88)
void  SqgSetTextureLookup(void *fn);          // 0x473700 (global 0x4c0f84)
void  WrlSetLogCallback(void *fn);            // 0x47a790 (global 0x4c3c4c)
void  WrlSetErrorCallback(void *fn);          // 0x47a7a0 (global 0x4c3c50)
