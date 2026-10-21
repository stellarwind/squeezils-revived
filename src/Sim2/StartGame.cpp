#include "Fn42d880/OverlayLifetime.h"
// SQZ.EXE 0x420170 (cdecl): start the game: clock settings from the main ini, seed the random generator, load the
// scripts, props, world, application, panel, land, bodies and clips (stepping the splash bar), then enter the start
// region, or the start place when StartInPlace says so or a place ini override is set. 0 when the panel fails.
#include <time.h>
#include <stdlib.h>
#include <string.h>
#pragma intrinsic(strcmp, strlen)
#include "IniReader/IniReader.h"

struct GameClock { GameClock(int rate, int maxSim, int maxRender, float multiplier); char d[0x30]; };   // 0x41eb90
struct World { World(); char d[0x10]; };                       // 0x4357b0
struct Application { Application(); ~Application(); char d[0xc4]; };   // 0x43a6a0 / 0x43a730
struct Panel { Panel(); int Load(); char d[0x6a8]; };   // 0x42d5f0 / 0x42d880, Load 0x42d9f0
struct Sim { virtual void v0(); virtual void Start(); };
struct LandSimulation : Sim { LandSimulation(); char d[0x14]; };   // 0x41e310
struct RegionSimulation : Sim { RegionSimulation(int region, int location); char d[0x64]; };   // 0x416e50
struct PlaceSimulation : Sim { PlaceSimulation(int region, int location, int flag); char d[0x40]; };   // 0x438850
struct Splash {
    virtual void SetProgress(float fraction);                  // slot 0
    virtual void Step();                                       // slot 1
};
extern Splash *g_splash;                                       // 0x4c75e8
extern GameClock *g_clock;                                     // 0x4badc0
extern World *g_world;                                         // 0x4bada8
extern Application *g_application;                             // 0x4badac
extern Panel *g_panel;                                         // 0x4badb0
extern LandSimulation *g_land;                                 // 0x4badb8
extern char g_placeIniOverride[];                              // 0x4c7348
extern const char kSimClockRateMS[], kMaxSimTimeMS[], kMaxRenderTimeMS[], kTimeMultiplier[];   // 0x4bae20 0x4bae10 0x4bae00 0x4badf0
extern const char kDefaults[], kStartRegion[], kStartLocation[], kStartInPlace[], kYes[];      // 0x4b9e60 0x4bade4 0x4badd4 0x4badc4 0x4b9e0c
void InitScriptGlobals();                                      // 0x422700
void ResetGame();                                              // 0x4482e0
void LoadFunctions(IniReader *ini);                            // 0x422730
void RunPass(IniReader *ini);                                  // 0x448300
void ReadPropFiles(IniReader *ini);                            // 0x429f40
int LoadProtoBodies();                                         // 0x458050
int LoadClips();                                               // 0x458590
void SetSimulation(Sim *s);                                    // 0x420670

int StartGame()
{
    IniReader ini;
    int rate = ini.GetInt(kSimClockRateMS, 50);
    int maxSim = ini.GetInt(kMaxSimTimeMS, 200);
    int maxRender = ini.GetInt(kMaxRenderTimeMS, 1000);
    float multiplier = ini.GetFloat(kTimeMultiplier, 0.0f);
    g_clock = new GameClock(rate, maxSim, maxRender, multiplier);
    time_t now;
    time(&now);
    srand((unsigned)now);
    g_splash->Step();
    InitScriptGlobals();
    g_splash->Step();
    ResetGame();
    g_splash->SetProgress(0.7f);
    LoadFunctions(&ini);
    g_splash->Step();
    RunPass(&ini);
    g_splash->SetProgress(0.8f);
    ReadPropFiles(&ini);
    g_splash->SetProgress(0.85f);
    g_world = new World;
    g_application = new Application;
    if (!g_panel) {
        g_panel = new Panel;
        if (!g_panel->Load()) {
            if (g_application)
                delete g_application;
            g_application = 0;
            if (g_panel)
                DestroyOverlay(g_panel);
            g_panel = 0;
            return 0;
        }
    }
    g_splash->SetProgress(0.9f);
    g_land = new LandSimulation;
    g_land->Start();
    g_splash->SetProgress(0.95f);
    LoadProtoBodies();
    g_splash->Step();
    LoadClips();
    g_splash->SetProgress(0.95f);
    ini.SetSection(kDefaults);
    int region = ini.GetInt(kStartRegion, 1);
    int location = ini.GetInt(kStartLocation, 1);
    Sim *sim;
    if (!strcmp(ini.GetString(kStartInPlace), kYes) || strlen(g_placeIniOverride))
        sim = new PlaceSimulation(region, location, 1);
    else
        sim = new RegionSimulation(region, location);
    SetSimulation(sim);
    sim->Start();
    return 1;
}
