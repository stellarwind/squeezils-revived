// PlaceSimulation: see PlaceSimulation.h. Reconstructed from SQZ.EXE 0x438850-0x43a000.
#include "PlaceSimulation.h"
#include "LandSimulation.h"
#include "RegionSimulation.h"
#include "NavSurface.h"
#include "CameraBody.h"
#include "SqueezilSoul.h"
#include "SqueezilRoboSoul.h"
#include "Panel.h"
#include "Splash.h"
#include "World.h"
#include "FrameProp.h"
#include "PropLoader.h"
#include "Resources.h"
#include "IniReader.h"
#include "Script.h"
#include "Original.h"
#include "VirtualCall.h"
#include <string.h>
#include <stdio.h>

static_assert(offsetof(PlaceSimulation, region) == 0x24, "PlaceSimulation layout");
static_assert(offsetof(PlaceSimulation, playerSoul) == 0x40, "PlaceSimulation layout");
static_assert(sizeof(PlaceLevel) == 0x2f4, "PlaceLevel layout");

static Splash **const kSplash = (Splash **)0x4c75e8;
static float *const kLodClipDistance = (float *)0x4bb908;
static char *const kPlaceIniOverride = (char *)0x4c7348;       // a place.ini named on the command line (empty: the place's own)
static void **const kFunctionOwner = (void **)0x4bae30;           // @00422bc0 the simulation the script functions belong to
static int *const kFoodNutsToFinish = (int *)0x4bad04;         // [Defaults] FoodNutsToFinish of the last place read
static const char kSaveSignature[] = "#SqzSave v1.1 ASCII\n";   // the pointer at 0x4bc6b0 holds this (the same text as the region's)


// The matrix rows of a body, recorded in a placement (the position, up and direction the next Place will use)
static void RecordPlacement(BodyPlacement *placement, SqueezilBody *body)
{
    const float *m = body->matrix;
    placement->position.x = m[12]; placement->position.y = m[13]; placement->position.z = m[14];
    placement->up.x = m[4]; placement->up.y = m[5]; placement->up.z = m[6];
    placement->direction.x = m[8]; placement->direction.y = m[9]; placement->direction.z = m[10];
}

static void Identity(float *m)
{
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// <region folder>\region\region.ini
static void RegionIniPath(char *path, int region)
{
    strcpy(path, GetLand()->GetPlaceName(region));
    strcat(path, "\\region\\region.ini");
}

// <region folder>\place<NN>\place.ini, or the override
static void PlaceIniPath(char *path, int region, int place)
{
    if (strlen(kPlaceIniOverride) == 0) {
        char number[16];
        strcpy(path, GetLand()->GetPlaceName(region));
        strcat(path, "\\place");
        sprintf(number, "%02d", place);
        strcat(path, number);
        strcat(path, "\\place.ini");
    } else {
        strcpy(path, kPlaceIniOverride);
    }
}

// @00438850
PlaceSimulation *PlaceSimulation::Construct(int regionNumber, int placeNumber, int locationNumber)
{
    LocationSimulation::Construct();
    Simulation::vtbl = (void **)0x49e224;
    region = regionNumber;
    place = placeNumber;
    location = locationNumber;
    return this;
}

// @004388f0
void PlaceSimulation::Destruct()
{
    Simulation::vtbl = (void **)0x49e224;
    LocationSimulation::Destruct();
}

// @004388d0
PlaceSimulation *PlaceSimulation::DestroyAndFree(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00438940 Loads the place's ini files and sets the scene up; Start then makes the player and the creatures. The progress bar of
// the splash screen: .05 .1 .15 .5 .55 .57 .59 .6 (and Start goes on to .65 .95 .97).
void PlaceSimulation::Init()
{
    LocationSimulation::Init();
    World *world = GetWorld();
    (*kSplash)->Load(1);
    (*kSplash)->SetProgress(0.05f);

    IniReader gameIni;
    *kLodClipDistance = gameIni.GetFloat("LODPropClipDistance", -1.0f);

    NavSurfaceList *list = (NavSurfaceList *)Operator_New(0x194);
    if (list)
        list->Construct();
    surfaces = list;
    surfaces->ReadSurfaceList(&gameIni);

    char path[256];
    RegionIniPath(path, region);
    IniReader regionIni(path);
    (*kSplash)->SetProgress(0.1f);
    ReadDefaultPropSprite(&regionIni);
    (*kSplash)->Step();
    ReadFunctionList(&regionIni);
    surfaces->ReadSurfaceList(&regionIni);
    (*kSplash)->SetProgress(0.15f);

    PlaceIniPath(path, region, place);
    IniReader placeIni(path);
    float clip = placeIni.GetFloat("LODPropClipDistance", -1.0f);
    if (*kLodClipDistance == -1.0f)
        *kLodClipDistance = clip;
    placeIni.SetSection("Type");
    levelType = placeIni.GetInt("LevelType", 0);
    (*kSplash)->SetProgress(0.5f);
    LoadResources(&placeIni);
    (*kSplash)->SetProgress(0.55f);
    ReadFunctionList(&placeIni);
    (*kSplash)->Step();
    *kFunctionOwner = this;
    world->CreateScene(&regionIni, &placeIni);
    (*kSplash)->SetProgress(0.57f);

    IniReader defaults;
    world->SetupLights(&defaults);
    world->SetupLights(&regionIni);
    world->SetupLights(&placeIni);

    placeIni.SetSection("Defaults");
    const char *midi = placeIni.GetString("MidiFile");
    if (strcmp(midi, "") != 0)
        StartMidi(midi);
    (*kSplash)->SetProgress(0.59f);

    // how far down a squeezil may fall, from the game's settings, then the place's
    gameIni.SetSection("Defaults");
    SetFloorLevelScript(gameIni.GetString("GroundPlaneFunction"));
    gameIni.SetSection("Physics");
    SetFloorLevel(gameIni.GetFloat("PlayerGroundPlane", -10.0f));
    (*kSplash)->SetProgress(0.6f);
    placeIni.SetSection("Defaults");
    const char *groundFunction = placeIni.GetString("GroundPlaneFunction");
    if (strlen(groundFunction) != 0)
        SetFloorLevelScript(groundFunction);
    float ground = placeIni.GetFloat("PlayerGroundPlane", -666.0f);
    if (ground != -666.0f)
        SetFloorLevel(ground);

    Start();
    (*kSplash)->Hide();
}

// @00438ee0
void PlaceSimulation::Start()
{
    if (!(*kSplash)->IsShown())
        (*kSplash)->Load(1);
    (*kSplash)->SetProgress(0.65f);
    finished = 0;

    char path[256];
    RegionIniPath(path, region);
    IniReader regionIni(path);
    PlaceIniPath(path, region, place);
    IniReader placeIni(path);
    surfaces->LoadGround(&placeIni);
    (*kSplash)->Step();
    (*kSplash)->SetProgress(0.95f);
    WorldReadPropFiles(&regionIni, &placeIni);
    (*kSplash)->Step();

    PlaceLevel *file = (PlaceLevel *)Operator_New(0x2f4);
    level = file ? PlaceLevelConstruct(file) : NULL;
    (*kSplash)->Step();
    PlaceLevelPrepare(level);
    PlaceLevelPrepare2(level);
    (*kSplash)->Step();
    int hasFile = PlaceLevelRead(level, &placeIni);

    // the player
    IniReader defaults;
    defaults.SetSection("Defaults");
    int color = ReadBodyColor(&defaults);
    float matrix[16];
    Identity(matrix);
    SqueezilBody *body = (SqueezilBody *)Operator_New(sizeof(SqueezilBody));
    if (body)
        body->Init(matrix, color);
    Possessions *possessions = GetLand()->Lend(0);
    if (hasFile) {
        if (level->loaded)
            body->Place(level->start);
        else {
            body->PlaceOnFace(&level->start.contact);
            RecordPlacement(&level->start, body);
        }
    } else {
        Contact first;
        memset(&first, 0, sizeof first);
        first.face = 0;
        first.second = 0;
        body->PlaceOnFace(&first);
        level->start.contact = first;
        RecordPlacement(&level->start, body);
    }
    (*kSplash)->Step();

    void *soul = Operator_New(0xa4);                          // SqueezilPlayerSoul
    if (soul)
        soul = PlayerSoulInit(soul, body, possessions, this);
    playerSoul = soul;
    *kFoodNutsToFinish = placeIni.GetInt("FoodNutsToFinish", *kFoodNutsToFinish);
    defaults.SetSection("Physics");
    SetMaxLandingSpeed(defaults.GetFloat("MaxLandingSpeed", 100.0f));
    float landing = placeIni.GetFloat("MaxLandingSpeed", -666.0f);
    if (landing != -666.0f)
        SetMaxLandingSpeed(landing);
    AddCreature(0, body, playerSoul);
    int playerId = 0;
    CallVirtual<int>(&level->ids, 1, &playerId, -1);

    CameraBody *cameraBody = (CameraBody *)Operator_New(0xc0);
    if (cameraBody)
        cameraBody->Init(matrix);
    void *cameraSoul = Operator_New(0x168);                   // PlaceCameraSoul
    cameraSoul = cameraSoul ? PlaceCameraSoulInit(cameraSoul, cameraBody, body, playerSoul) : NULL;
    AddCreature(1, cameraBody, cameraSoul);

    char *app = (char *)GetApplication();
    *(void **)(app + 0x18) = playerSoul ? (char *)playerSoul + 0x80 : NULL;
    *(void **)(app + 0x1c) = cameraSoul ? (char *)cameraSoul + 4 : NULL;
    ((Panel *)GetPanel())->ShowStatus(*kFoodNutsToFinish);
    *(int *)((char *)GetApplication() + 0xb4) = 1;            // @0043aa10
    (*kSplash)->Step();

    // the creatures the file puts in the place
    if (hasFile) {
        for (int i = 0; i < level->creatures.count; i++) {
            Possessions *theirs = NewPossessions();
            LevelCreature *entry = level->creatures.At(i);
            void *info = PlaceLevelFindCreature(level, entry->name);
            SqueezilBody *creatureBody = (SqueezilBody *)Operator_New(sizeof(SqueezilBody));
            if (creatureBody)
                creatureBody->Init(matrix, *(int *)((char *)info + 0x20));
            if (level->loaded) {
                creatureBody->Place(*(BodyPlacement *)entry);
            } else {
                creatureBody->PlaceOnFace(&entry->contact);
                RecordPlacement((BodyPlacement *)entry, creatureBody);
            }
            void *creatureSoul = Operator_New(0xfc);          // SqueezilRoboSoul
            if (creatureSoul)
                creatureSoul = RoboSoulInit(creatureSoul, creatureBody, theirs, this, info);
            ((SqueezilRoboSoul *)creatureSoul)->levelIndex = i;
            int id = NewId();
            AddCreature(id, creatureBody, creatureSoul);
            CallVirtual<int>(&level->ids, 1, &id, -1);
        }
    }
    (*kSplash)->Step();
    if (hasFile)
        PlaceLevelFinish(level);
    (*kSplash)->SetProgress(0.97f);
    (*kSplash)->Hide();
    RunFrame();
    ResetClock();
    ((Panel *)GetPanel())->Update(1);

    ScriptCaller caller;
    caller.unknown0 = 0;
    caller.unknown4 = (int)playerSoul;
    caller.soul = playerSoul;
    caller.unknownC = 0;
    caller.actorIndex = ((LocationSimulation *)GetCurrentSimulation())->FindIdBySoul(playerSoul);
    caller.prop = NULL;
    PostFunction("StartAmbient", NULL, 0, 1);
    PostFunction("StartFunction", &caller, 0, 1);

    if (*(int *)((char *)GetApplication() + 0x78))
        ApplicationSetInputMode(GetApplication(), 1);
    if (*(int *)0x4bd964 == 0)
        StartAudio();
    else
        *(int *)0x4bd964 = 0;
}

// @00439a00
void PlaceSimulation::ClearCreatures()
{
    while (AudioUpdate() > 0) {
    }
    surfaces->UnloadLastFile();
    NavSurfaceDestroyInstances(surfaces);
    if (level) {
        PlaceLevelDestruct(level);
        Operator_Delete(level);
    }
    ReleaseFunctionTable();
    ClearWorldObjects();
    LocationSimulation::ClearCreatures();
}

// @00439a50 (the repeats are the original's)
void PlaceSimulation::Clear()
{
    CallVirtual<void>(this, 7);                 // ClearCreatures
    while (AudioUpdate() > 0) {
    }
    UnloadSounds();
    if (*(int *)0x4bf458)
        StopMidi();
    NavSurfaceDestroyLoaded(surfaces);
    *kFunctionOwner = NULL;
    ReleaseFunctions();
    UnloadResources();
    surfaces->UnloadLastFile();
    ReleaseFunctions();
    surfaces->UnloadLastFile();
    if (surfaces) {
        surfaces->Destruct();
        Operator_Delete(surfaces);
    }
    LocationSimulation::Clear();
}

// @00439ae0
void PlaceSimulation::CollideCreatures()
{
    surfaces->MoveCarried();
    LocationSimulation::CollideCreatures();
}

// @00439b00
void PlaceSimulation::UpdateSouls()
{
    if (finished) {
        PlayerSoulFinishedUpdate(playerSoul);
        return;
    }
    LocationSimulation::UpdateSouls();
    PlaceLevelUpdate(level);
    PlayerSoulPostUpdate(playerSoul);
    surfaces->FlushRemoved();
}

// @00439e90
Float3 *PlaceSimulation::Velocity(Float3 *out, void *body)
{
    if (!CallVirtual<int>(body, BODY_IS_ON_SURFACE)) {
        out->x = out->y = out->z = 0;
        return out;
    }
    Contact *contact = CallVirtual<Contact *>(body, BODY_GET_CONTACT);
    InstancedSurface *face = (InstancedSurface *)NavSurfaceFaceOf(GetSurfaces(), contact);
    float *matrix = CallVirtual<float *>(body, BODY_GET_MATRIX);
    face->Carried((float *)out, matrix + 12);
    return out;
}

// @00439b70 Loads a saved game, like RegionSimulation::Load (the file has the same shape; a place does not save, see CanSave)
int PlaceSimulation::Load(const char *path)
{
    void *file = GameFopen(path, "r");
    if (!file)
        return 0;
    char line[256];
    if (!GameFgets(line, 255, file)) {
        GameFclose(file);
        return 0;
    }
    if (strcmp(line, kSaveSignature) != 0) {
        GameFclose(file);
        return 0;
    }
    if (!GameFgets(line, 255, file)) {
        GameFclose(file);
        return 0;
    }
    int newRegion, newLocation;
    GameSscanf(line, "%d %d", &newRegion, &newLocation);

    Inventory *inventory = (Inventory *)Operator_New(0x70);
    if (inventory)
        InventoryConstruct(inventory);
    if (!InventoryLoad(inventory, file)) {
        GameFclose(file);
        return 0;
    }
    GameFclose(file);

    void *player = DynamicCast(((LocationSimulation *)GetCurrentSimulation())->GetSoul(0), kSoulType, kSqueezilPlayerSoulType);
    Possessions *possessions = CallVirtual<Possessions *>(player, SQS_TAKE_POSSESSIONS);
    PossessionsSetInventory(possessions, inventory);
    GetLand()->Return(0, possessions);

    void *current = GetCurrentSimulation();
    CallVirtual<void>(current, 2);                                   // Clear
    current = GetCurrentSimulation();
    if (current)
        CallVirtual<void>(current, 0, 1);
    RegionSimulation *next = (RegionSimulation *)Operator_New(0x68);
    if (next)
        next->Construct(newRegion, newLocation);
    SetCurrentSimulation(next);
    CallVirtual<void>(next, 1);                                      // Init
    ((Panel *)GetPanel())->UpdateRegionSprites();
    SetGamePaused(1);
    return 1;
}

// @00439db0
int PlaceSimulation::Save(const char *path)
{
    void *file = GameFopen(path, "w");
    if (!file)
        return 0;
    if (GameFprintf(file, "%s", kSaveSignature) < 0) {
        GameFclose(file);
        return 0;
    }
    if (GameFprintf(file, "%d %d\n", region, location) < 0) {
        GameFclose(file);
        return 0;
    }
    void *player = DynamicCast(((LocationSimulation *)GetCurrentSimulation())->GetSoul(0), kSoulType, kSqueezilPlayerSoulType);
    Possessions *possessions = CallVirtual<Possessions *>(player, SQS_GET_POSSESSIONS);
    if (!InventorySave(possessions->inventory, file)) {
        GameFclose(file);
        return 0;
    }
    GameFclose(file);
    return 1;
}
