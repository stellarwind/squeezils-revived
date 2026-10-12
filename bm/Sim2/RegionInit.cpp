// RegionSimulation::Init (SQZ.EXE 0x416f50): read region.ini, build the scene, make the player (body + soul) and the camera. Draft.
#include "IniReader/IniReader.h"
#include "String/String.h"
#include <string.h>
#include <stdio.h>
struct Float3 { float x, y, z; Float3() {} ~Float3(); };   // dtor 0x401660
struct Matrix {
    float m[16];
    Matrix() {}
    Matrix(const Float3 &t);                             // 0x47e330
    void Identity() {
        m[0] = 1; m[1] = 0; m[2] = 0; m[3] = 0;
        m[4] = 0; m[5] = 1; m[6] = 0; m[7] = 0;
        m[8] = 0; m[9] = 0; m[10] = 1; m[11] = 0;
        m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
    }
    ~Matrix();                                           // 0x402310
};
struct SqueezilBody { char pad[0x238]; SqueezilBody(Matrix *m, int color); };   // 0x457050
struct CameraBody { char pad[0xc0]; CameraBody(Matrix *m); };   // 0x40b8a0
struct RegionSoul { char pad[0xe8]; RegionSoul(SqueezilBody *b, void *possessions, void *sim); void StartFunction(); };   // 0x436710 / 0x436a60
struct CameraSoul { char pad[0x168]; CameraSoul(CameraBody *c, SqueezilBody *b, RegionSoul *p); };  // 0x409d10
struct Possessions;
struct Splash {
    virtual void SetProgress(float p);
    virtual void Step();
    int Load(int which);                                 // 0x44ea30
    void Hide();                                         // 0x44ee40
};
extern Splash *g_splash;                                 // 0x4c75e8
extern float g_lodClip;                                  // 0x4bb908
extern int g_audioFlag;                                  // 0x4bd964
struct World {
    void CreateScene(IniReader *region, IniReader *place);   // 0x435910
    void ReadPropFiles(IniReader *region, IniReader *place); // 0x436370
    void SetupLights(IniReader *ini);                    // 0x436040
};
World *GetWorld();                                       // 0x4206a0
struct Land { const char *GetPlaceName(int index);       // 0x41e8c0
              virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
              virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
              virtual Possessions *Lend(int id); };
Land *GetLand();                                         // 0x420680
struct Application { char pad[0x18]; void *playerInput; void *cameraInput; char pad2[0x78 - 0x20]; int hasInput; void SetInputMode(int m); void SetFlag(); void FlushInput(); };   // 0x43b980 / 0x43aa20 / 0x43c970
Application *GetApplication();                           // 0x4206b0
struct Panel { void ShowQuest(int region); void Update(int n); };   // 0x42da60 / 0x42db10
Panel *GetPanel();                                       // 0x4206c0
struct Sim { int FindIdBySoul(void *soul); };            // 0x4140b0
Sim *GetSimulation();                                    // 0x420690
struct ScriptCaller { char pad[0x18]; ScriptCaller(void *soul, int id, int zero); };   // 0x420bc0
void PostFunction(const char *name, ScriptCaller *caller, int a, int b);   // 0x422ce0 (cdecl)
int ReadBodyColor(IniReader *ini);                       // 0x458f60 (cdecl)
void GameLog(const char *fmt, ...);                      // 0x419290 (cdecl)
void ReadFunctionList(IniReader *ini);                   // 0x422730 (cdecl)
void LoadResources(IniReader *ini);                      // 0x448300 (cdecl)
void *FindPropTemplate(String name, int a, int b);       // 0x42c8b0 (cdecl)
int StartMidi(const char *file);                         // 0x45aa00 (cdecl)
void RunFrame();                                         // 0x4206e0 (cdecl)
void ResetClock();                                       // 0x420760 (cdecl)
void StartAudio();                                       // 0x44add0 (cdecl)
struct SpotInfo { Float3 position; };
struct RegionSimulation {
    char pad0[0x24];
    int region, place;                                   // 0x24 0x28
    int location;                                        // 0x2c
    char pad1[0x58 - 0x30];
    void *unblurredTree, *unblurredNutTree, *blurredTree, *blurredNutTree;   // 0x58..0x64
    void LocationInit();                                 // 0x412f90
    void ReadLocations(IniReader *ini);                  // 0x417680
    SpotInfo *GetPlace(int index);                       // 0x418140
    int GetRegion();                                     // 0x418160
    void AddCreature(int id, void *actor, void *soul);   // 0x414160
    void Init();
};

void RegionSimulation::Init()
{
    g_splash->Load(2);
    LocationInit();
    IniReader gameIni;
    g_lodClip = gameIni.GetFloat("LODPropClipDistance", -1.0f);
    World *world = GetWorld();
    location = 0;
    char path[128];
    strcpy(path, GetLand()->GetPlaceName(region));
    strcat(path, "\\region\\region.ini");
    GameLog("NOTE: RegionSimulation::initialize: loading '%s'\n", path);
    IniReader regionIni(path);
    float clip = regionIni.GetFloat("LODPropClipDistance", -1.0f);
    if (g_lodClip == -1.0f)
        g_lodClip = clip;
    g_splash->SetProgress(0.35f);
    ReadFunctionList(&regionIni);
    g_splash->Step();
    LoadResources(&regionIni);
    g_splash->SetProgress(0.45f);
    world->CreateScene(&regionIni, 0);
    g_splash->SetProgress(0.55f);
    world->ReadPropFiles(&regionIni, 0);
    g_splash->SetProgress(0.65f);

    regionIni.SetSection("Defaults");
    const char *t1 = regionIni.GetString("UnblurredTreeSprite");
    unblurredTree = FindPropTemplate(String(t1), 0, 0);
    const char *t2 = regionIni.GetString("UnblurredTreeSpriteNut");
    unblurredNutTree = FindPropTemplate(String(t2), 0, 0);
    const char *t3 = regionIni.GetString("BlurredTreeSprite");
    blurredTree = FindPropTemplate(String(t3), 0, 0);
    const char *t4 = regionIni.GetString("BlurredTreeSpriteNut");
    blurredNutTree = FindPropTemplate(String(t4), 0, 0);
    ReadLocations(&regionIni);
    g_splash->SetProgress(0.75f);

    IniReader defaults;
    GetWorld()->SetupLights(&defaults);
    GetWorld()->SetupLights(&regionIni);
    Float3 start = GetPlace(location)->position;
    defaults.SetSection("Defaults");
    Matrix matrix(start);
    SqueezilBody *body = new SqueezilBody(&matrix, ReadBodyColor(&defaults));
    GameLog("Done with geometry.  Initializing soul: ");
    g_splash->SetProgress(0.85f);
    Possessions *possessions = GetLand()->Lend(0);
    RegionSoul *soul = new RegionSoul(body, possessions, this);
    g_splash->SetProgress(0.9f);
    GameLog("OK.\nInitializing camera: ");
    AddCreature(0, body, soul);
    matrix.Identity();
    CameraBody *cameraBody = new CameraBody(&matrix);
    CameraSoul *cameraSoul = new CameraSoul(cameraBody, body, soul);
    AddCreature(1, cameraBody, cameraSoul);
    GetApplication()->playerInput = soul ? (char *)soul + 0x80 : 0;
    GetApplication()->cameraInput = cameraSoul ? (char *)cameraSoul + 4 : 0;
    GetPanel()->ShowQuest(GetRegion());
    GetApplication()->SetFlag();

    regionIni.SetSection("Defaults");
    const char *midi = regionIni.GetString("MidiFile");
    if (strcmp(midi, "") != 0 && !StartMidi(midi))
        GameLog("Failure in midi manager initialization\n");
    g_splash->Step();
    g_splash->Hide();
    RunFrame();
    ResetClock();
    GetPanel()->Update(1);

    ScriptCaller caller(soul, GetSimulation()->FindIdBySoul(soul), 0);
    PostFunction("StartFunction", &caller, 0, 0);
    soul->StartFunction();
    if (GetApplication()->hasInput)
        GetApplication()->SetInputMode(1);
    if (g_audioFlag == 0)
        StartAudio();
    else
        g_audioFlag = 0;
    GetApplication()->FlushInput();
    GameLog("OK.\n");
}
