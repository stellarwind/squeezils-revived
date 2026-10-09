// Globals: the game's global-object getters/setters.
#include "Globals/Globals.h"

extern void *g_land;               // 0x4badb8
extern void *g_world;              // 0x4bada8
extern void *g_currentSimulation;  // 0x4badbc
extern void *g_application;        // 0x4badac
extern void *g_panel;              // 0x4badb0
extern void *g_sqgErrorCallback;   // 0x4c0f88
extern void *g_sqgTextureLookup;   // 0x4c0f84
extern void *g_wrlLogCallback;     // 0x4c3c4c
extern void *g_wrlErrorCallback;   // 0x4c3c50

void *GetLand()              { return g_land; }
void *GetWorld()             { return g_world; }
void *GetCurrentSimulation() { return g_currentSimulation; }
void *GetApplication()       { return g_application; }
void *GetPanel()             { return g_panel; }

void SetCurrentSimulation(void *simulation) { g_currentSimulation = simulation; }
void SqgSetErrorCallback(void *fn)          { g_sqgErrorCallback = fn; }
void SqgSetTextureLookup(void *fn)          { g_sqgTextureLookup = fn; }
void WrlSetLogCallback(void *fn)            { g_wrlLogCallback = fn; }
void WrlSetErrorCallback(void *fn)          { g_wrlErrorCallback = fn; }
