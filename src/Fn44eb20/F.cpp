#include "Runtime/DynamicCast.h"
// 0x44eb20 (ret 4): Splash::FindImage - pick the ini of the game (0), of the current place (1) or of the current region (2)
// and build the splash bitmap path ".\ui\<Defaults/Splash or "splash">.bmp" from it.
#include <stdio.h>
#include <string.h>
#include "IniReader/IniReader.h"
extern char g_iniPath[];                                    // 0x4c74d8
extern const char kWorldSimulationType[], kWorldPlaceType[], kWorldRegionType[];   // 0x4ba1d0, 0x4bae38, 0x4ba370
struct Simulation;
Simulation *GetSimulation();                                // 0x420690
struct PlaceSim { char pad[0x24]; int region; int location; };   // 0x24 / 0x28
struct RegionSim { int GetRegion(); };                      // 0x418160
struct Land { const char *GetPlaceName(int index); };       // 0x41e8c0
Land *GetLand();                                            // 0x420680

struct Splash {
    char pad[0x20];
    char file[0x100];                                       // 0x20
    void FindImage(int which);
};

void Splash::FindImage(int which)
{
    char path[60];
    char number[20];
    switch (which) {
    case 0:
        strcpy(path, g_iniPath);
        break;
    case 1: {
        PlaceSim *place = (PlaceSim *)SqzRTDynamicCast(GetSimulation(), 0, kWorldSimulationType, kWorldPlaceType, 0);
        strcpy(path, GetLand()->GetPlaceName(place->region));
        strcat(path, "\\place");
        sprintf(number, "%02d", place->location);
        strcat(path, number);
        strcat(path, "\\place.ini");
        break;
    }
    case 2: {
        RegionSim *region = (RegionSim *)SqzRTDynamicCast(GetSimulation(), 0, kWorldSimulationType, kWorldRegionType, 0);
        strcpy(path, GetLand()->GetPlaceName(region->GetRegion()));
        strcat(path, "\\region\\region.ini");
        break;
    }
    }
    IniReader ini(path);
    strcpy(file, ".\\ui\\");
    ini.SetSection("Defaults");
    const char *name = ini.GetString("Splash");
    strcat(file, strlen(name) ? name : "splash");
    strcat(file, ".bmp");
}
