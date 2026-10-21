// PlaceSimulation::Start (SQZ.EXE 0x438ee0): read the level file, make the player (body + soul), the camera and the file's creatures.
#include "IniReader/IniReader.h"
#include "String/String.h"
#include <string.h>
#include <stdio.h>
struct Float3 { float x, y, z; void Get(const float *m, int i) { x = m[i]; y = m[i + 1]; z = m[i + 2]; } };
struct Matrix {                                          // identity on construction, out-of-line dtor 0x402310
    float m[16];
    Matrix() {}
    void Identity() {
        m[0] = 1; m[1] = 0; m[2] = 0; m[3] = 0;
        m[4] = 0; m[5] = 1; m[6] = 0; m[7] = 0;
        m[8] = 0; m[9] = 0; m[10] = 1; m[11] = 0;
        m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
    }
    ~Matrix();
};
struct Contact {                                         // 0x48; dtor 0x4031e0
    int face, second;
    Matrix matrix;
    Contact() { face = 0; second = 0; }
    ~Contact();
};
struct RawContact {
    int w[18];
    void Set(const Contact *c) {
        const int *s = (const int *)c;
        w[0]=s[0]; w[1]=s[1]; w[2]=s[2]; w[3]=s[3]; w[4]=s[4]; w[5]=s[5]; w[6]=s[6]; w[7]=s[7]; w[8]=s[8];
        w[9]=s[9]; w[10]=s[10]; w[11]=s[11]; w[12]=s[12]; w[13]=s[13]; w[14]=s[14]; w[15]=s[15]; w[16]=s[16]; w[17]=s[17];
    }
};
struct BodyPlacement {                                   // 0xb0; copy ctor 0x4020b0, dtor 0x402250
    int unknown00;
    float matrix[16];
    RawContact contact;
    Float3 position, up, direction;
    BodyPlacement(const BodyPlacement &o);
    ~BodyPlacement();
};
struct LevelCreature {                                   // 0xb0
    int kind;
    char name[64];
    RawContact contact;
    Float3 position, up, direction;
};
struct Mat { Float3 r0; float w0; Float3 r1; float w1; Float3 r2; float w2; Float3 pos; float w3; };
struct Body {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual float *GetMatrix();                          // slot 3 (+0xc)
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
    virtual void v9(); virtual void v10(); virtual void v11(); virtual void v12();
    virtual void PlaceOnFace(void *contact);             // slot 13 (+0x34)
};
struct SqueezilBody : Body {
    char pad[0x238 - 4];
    SqueezilBody(Matrix *m, int color);                  // 0x457050
    void Place(BodyPlacement p);                         // 0x4577e0
};
struct CameraBody { char pad[0xc0]; CameraBody(Matrix *m); };   // 0x40b8a0
struct PlayerSoul { char pad[0xa4]; PlayerSoul(SqueezilBody *b, void *possessions, void *sim); };   // 0x41ee30
struct CameraSoul { char pad[0x168]; CameraSoul(CameraBody *c, SqueezilBody *b, PlayerSoul *p); };  // 0x40a630
struct RoboSoul { char pad[0x98]; int levelIndex; char pad2[0xfc - 0x9c]; RoboSoul(SqueezilBody *b, void *possessions, void *sim, void *info); };   // 0x4043a0
struct Possessions { char pad[8]; Possessions(); };      // 0x4488d0
template <class T> struct Arr {
    virtual T Get(int i);                                // slot 0
    virtual int Insert(const T &v, int at);              // slot 1
    int count;
    int data, cap;
};
struct PlaceLevel {                                      // 0x2f4; ctor 0x44af30
    int loaded;
    char pad[0x1f4];
    BodyPlacement start;                                 // 0x1f8
    Arr<LevelCreature *> creatures;                      // 0x2a8
    Arr<int> ids;                                        // 0x2b8
    char pad2[0x2f4 - 0x2c8];
    PlaceLevel();
    void Prepare();                                      // 0x44b950
    void Prepare2();                                     // 0x44b780
    int Read(IniReader *ini);                            // 0x44bd60
    void *FindCreature(String name);                     // 0x44e1b0
    void Finish();                                       // 0x44df00
    BodyPlacement *StartPlacement();                     // 0x439f70
};
struct Splash {
    virtual void SetProgress(float p);
    virtual void Step();
    int Load(int which);                                 // 0x44ea30
    int IsShown();                                       // 0x44eea0
    void Hide();                                         // 0x44ee40
};
extern Splash *g_splash;                                 // 0x4c75e8
extern int g_foodNuts;                                   // 0x4bad04
extern char g_placeIniOverride[];                        // 0x4c7348
extern int g_audioFlag;                                  // 0x4bd964
struct World { void ReadPropFiles(IniReader *region, IniReader *place); };   // 0x436370 (ret 8)
World *GetWorld();                                       // 0x4206a0
struct Land { const char *GetPlaceName(int index);       // 0x41e8c0
              virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
              virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
              virtual Possessions *Lend(int id); };      // slot 10 (+0x28)
Land *GetLand();                                         // 0x420680
struct NavSurfaceList { void LoadGround(IniReader *ini); };   // 0x40e480
// Application local field view.
struct Application {
    char pad[0x18];
    void *playerInput; // 0x18
    void *cameraInput; // 0x1c
    char pad2[0x78 - 0x20];
    int hasInput; // 0x78
    char pad3[0xb4 - 0x7c];
    int flag; // 0xb4
    void SetInputMode(int m); // 0x43b980
    void SetFlag(); // 0x43aa10: set the application flag at +0xb4
};
Application *GetApplication();                           // 0x4206b0
struct Panel { void ShowStatus(int n); void Update(int n); };   // 0x42dac0 / 0x42db10
Panel *GetPanel();                                       // 0x4206c0
struct Sim { int FindIdBySoul(void *soul); };            // 0x4140b0
Sim *GetSimulation();                                    // 0x420690
struct ScriptCaller { char pad[0x18]; ScriptCaller(void *soul, int id, int zero); };   // 0x420900
void PostFunction(const char *name, ScriptCaller *caller, int a, int b);   // 0x422ce0 (cdecl)
int ReadBodyColor(IniReader *ini);                       // 0x458f60 (cdecl)
void SetMaxLandingSpeed(float v);                        // 0x450c10 (cdecl)
void RunFrame();                                         // 0x4206e0 (cdecl)
void ResetClock();                                       // 0x420760 (cdecl)
void StartAudio();                                       // 0x44add0 (cdecl)
struct PlaceSimulation {
    char pad0[0x24];
    int region, place, location;                         // 0x24..
    int levelType;                                       // 0x30
    int finished;                                        // 0x34
    PlaceLevel *level;                                   // 0x38
    NavSurfaceList *surfaces;                            // 0x3c
    PlayerSoul *playerSoul;                              // 0x40
    void AddCreature(int id, void *actor, void *soul);   // 0x414160
    int NewId();                                         // 0x414250
    void Start();
};

void PlaceSimulation::Start()
{
    if (!g_splash->IsShown())
        g_splash->Load(1);
    g_splash->SetProgress(0.65f);
    finished = 0;

    char path[128];
    strcpy(path, GetLand()->GetPlaceName(region));
    strcat(path, "\\region\\region.ini");
    IniReader regionIni(path);
    if (strlen(g_placeIniOverride) == 0) {
        strcpy(path, GetLand()->GetPlaceName(region));
        strcat(path, "\\place");
        char number[10];
        sprintf(number, "%d", place);
        strcat(path, number);
        strcat(path, "\\place.ini");
    } else {
        strcpy(path, g_placeIniOverride);
    }
    IniReader placeIni(path);
    surfaces->LoadGround(&placeIni);
    g_splash->Step();
    g_splash->SetProgress(0.95f);
    GetWorld()->ReadPropFiles(&regionIni, &placeIni);
    g_splash->Step();

    level = new PlaceLevel;
    g_splash->Step();
    level->Prepare();
    level->Prepare2();
    g_splash->Step();
    int hasFile = level->Read(&placeIni);

    IniReader defaults;
    defaults.SetSection("Defaults");
    int color = ReadBodyColor(&defaults);
    Matrix matrix;
    matrix.Identity();
    SqueezilBody *body = new SqueezilBody(&matrix, color);
    Possessions *possessions = GetLand()->Lend(0);
    if (hasFile) {
        if (level->loaded) {
            body->Place(*level->StartPlacement());
        } else {
            body->PlaceOnFace(&level->start.contact);
            level->start.position.Get(body->GetMatrix(), 12);
            level->start.up.Get(body->GetMatrix(), 4);
            level->start.direction.Get(body->GetMatrix(), 8);
        }
    } else {
        Contact first;
        body->PlaceOnFace(&first);
        level->start.contact.Set(&first);
        level->start.position.Get(body->GetMatrix(), 12);
        level->start.up.Get(body->GetMatrix(), 4);
        level->start.direction.Get(body->GetMatrix(), 8);
    }
    g_splash->Step();

    playerSoul = new PlayerSoul(body, possessions, this);
    g_foodNuts = placeIni.GetInt("FoodNutsToFinish", g_foodNuts);
    defaults.SetSection("Physics");
    SetMaxLandingSpeed(defaults.GetFloat("MaxLandingSpeed", 100.0f));
    float landing = placeIni.GetFloat("MaxLandingSpeed", -666.0f);
    if (landing != -666.0f)
        SetMaxLandingSpeed(landing);
    AddCreature(0, body, playerSoul);
    level->ids.Insert(0, -1);

    CameraBody *cameraBody = new CameraBody(&matrix);
    CameraSoul *cameraSoul = new CameraSoul(cameraBody, body, playerSoul);
    AddCreature(1, cameraBody, cameraSoul);

    void *playerIn = playerSoul ? (char *)playerSoul + 0x80 : 0;
    GetApplication()->playerInput = playerIn;
    void *cameraIn = cameraSoul ? (char *)cameraSoul + 4 : 0;
    GetApplication()->cameraInput = cameraIn;
    GetPanel()->ShowStatus(g_foodNuts);
    GetApplication()->SetFlag();
    g_splash->Step();

    if (hasFile) {
        for (int i = 0; i < level->creatures.count; i++) {
            Possessions *theirs = new Possessions;
            LevelCreature *entry = level->creatures.Get(i);
            void *info = level->FindCreature(String(entry->name));
            SqueezilBody *creatureBody = new SqueezilBody(&matrix, *(int *)((char *)info + 0x20));
            if (!level->loaded) {
                creatureBody->PlaceOnFace(&entry->contact);
                entry->position.Get(creatureBody->GetMatrix(), 12);
                entry->up.Get(creatureBody->GetMatrix(), 4);
                entry->direction.Get(creatureBody->GetMatrix(), 8);
            } else
                creatureBody->Place(*(BodyPlacement *)entry);
            RoboSoul *creatureSoul = new RoboSoul(creatureBody, theirs, this, info);
            creatureSoul->levelIndex = i;
            int id = NewId();
            AddCreature(id, creatureBody, creatureSoul);
            level->ids.Insert(id, -1);
        }
    }
    g_splash->Step();
    if (hasFile)
        level->Finish();
    g_splash->SetProgress(0.97f);
    g_splash->Hide();
    RunFrame();
    ResetClock();
    GetPanel()->Update(1);

    ScriptCaller caller(playerSoul, GetSimulation()->FindIdBySoul(playerSoul), 0);
    PostFunction("StartAmbient", 0, 0, 1);
    PostFunction("StartFunction", &caller, 0, 1);
    if (GetApplication()->hasInput)
        GetApplication()->SetInputMode(1);
    if (g_audioFlag == 0)
        StartAudio();
    else
        g_audioFlag = 0;
}
