// PlaceSimulation slot 1 (SQZ.EXE 0x438940): read the ini files (game, region, place), build the scene and the surfaces, start the music.
#include "IniReader/IniReader.h"
#include <string.h>
#include <stdio.h>
struct Float3 { float x, y, z; Float3() {} ~Float3(); };   // dtor 0x401660 (five unused locals keep their EH states)
struct Splash {
    virtual void SetProgress(float p);                   // slot 0
    virtual void Step();                                 // slot 1
    int Load(int which);                                 // 0x44ea30
    void Hide();                                         // 0x44ee40
};
extern Splash *g_splash;                                 // 0x4c75e8
extern float g_lodClip;                                  // 0x4bb908
extern char g_placeIniOverride[];                        // 0x4c7348
extern char g_empty[];                                   // 0x4b9488
struct World {
    void CreateScene(IniReader *region, IniReader *place);   // 0x435910 (ret 8)
    void SetupLights(IniReader *ini);                    // 0x436040 (ret 4)
};
World *GetWorld();                                       // 0x4206a0 (cdecl)
struct Land { const char *GetPlaceName(int index); };    // 0x41e8c0
Land *GetLand();                                         // 0x420680 (cdecl)
struct NavSurfaceList { char pad[0x194]; NavSurfaceList(); void ReadSurfaceList(IniReader *ini); };   // ctor 0x40d530, 0x40d870
void ReadDefaultPropSprite(IniReader *ini);              // 0x425870 (cdecl)
void ReadFunctionList(IniReader *ini);                   // 0x422730 (cdecl)
void LoadResources(IniReader *ini);                      // 0x448300 (cdecl)
void SetFunctionOwner(void *owner);                      // 0x422bc0 (cdecl)
void StartMidi(const char *file);                        // 0x45aa00 (cdecl)
void SetFloorLevelScript(const char *name);              // 0x450b90 (cdecl)
void SetFloorLevel(float level);                         // 0x450c00 (cdecl)
struct PlaceSimulation {
    char pad0[0x24];
    int region;                                          // 0x24
    int place;                                           // 0x28
    int location;                                        // 0x2c
    int levelType;                                       // 0x30
    int finished;                                        // 0x34
    void *level;                                         // 0x38
    NavSurfaceList *surfaces;                            // 0x3c
    void *playerSoul;                                    // 0x40
    void LocationInit();                                 // 0x412f90
    void Start();                                        // 0x438ee0
    void Init();                                         // slot 1
};

void PlaceSimulation::Init()
{
    LocationInit();
    Float3 f1, f2, f3, f4, f5;
    World *world = GetWorld();
    g_splash->Load(1);
    g_splash->SetProgress(0.05f);

    IniReader gameIni;
    g_lodClip = gameIni.GetFloat("LODPropClipDistance", -1.0f);
    surfaces = new NavSurfaceList;
    surfaces->ReadSurfaceList(&gameIni);

    char path[128];
    strcpy(path, GetLand()->GetPlaceName(region));
    strcat(path, "\region\region.ini");
    IniReader regionIni(path);
    g_splash->SetProgress(0.1f);
    ReadDefaultPropSprite(&regionIni);
    g_splash->Step();
    ReadFunctionList(&regionIni);
    surfaces->ReadSurfaceList(&regionIni);
    g_splash->SetProgress(0.15f);

    if (strlen(g_placeIniOverride) == 0) {
        strcpy(path, GetLand()->GetPlaceName(region));
        strcat(path, "\place");
        char number[10];
        sprintf(number, "%d", place);
        strcat(path, number);
        strcat(path, "\place.ini");
    } else {
        strcpy(path, g_placeIniOverride);
    }
    IniReader placeIni(path);
    float clip = placeIni.GetFloat("LODPropClipDistance", -1.0f);
    if (g_lodClip == -1.0f)
        g_lodClip = clip;
    placeIni.SetSection("Type");
    levelType = placeIni.GetInt("LevelType", 0);
    g_splash->SetProgress(0.5f);
    LoadResources(&placeIni);
    g_splash->SetProgress(0.55f);
    ReadFunctionList(&placeIni);
    g_splash->Step();
    SetFunctionOwner(this);
    world->CreateScene(&regionIni, &placeIni);
    g_splash->SetProgress(0.57f);

    IniReader defaults;
    GetWorld()->SetupLights(&defaults);
    GetWorld()->SetupLights(&regionIni);
    GetWorld()->SetupLights(&placeIni);

    placeIni.SetSection("Defaults");
    const char *midi = placeIni.GetString("MidiFile");
    if (strcmp(midi, g_empty) != 0)
        StartMidi(midi);
    g_splash->SetProgress(0.59f);

    gameIni.SetSection("Defaults");
    SetFloorLevelScript(gameIni.GetString("GroundPlaneFunction"));
    gameIni.SetSection("Physics");
    SetFloorLevel(gameIni.GetFloat("PlayerGroundPlane", -10.0f));
    g_splash->SetProgress(0.6f);
    placeIni.SetSection("Defaults");
    const char *ground = placeIni.GetString("GroundPlaneFunction");
    if (strlen(ground) != 0)
        SetFloorLevelScript(ground);
    float level = placeIni.GetFloat("PlayerGroundPlane", -666.0f);
    if (level != -666.0f)
        SetFloorLevel(level);

    Start();
    g_splash->Hide();
}
