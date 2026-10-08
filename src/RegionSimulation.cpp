// RegionSimulation: see RegionSimulation.h. Reconstructed from SQZ.EXE 0x416c60-0x418a60.
#include "RegionSimulation.h"
#include "LandSimulation.h"
#include "SqueezilBody.h"
#include "SqueezilRegionSoul.h"
#include "CameraBody.h"
#include "Panel.h"
#include "Splash.h"
#include "Scene.h"
#include "World.h"
#include "Prop.h"
#include "PropLoader.h"
#include "FrameProp.h"
#include "Script.h"
#include "Original.h"
#include "VirtualCall.h"
#include <string.h>
#include <stdlib.h>

static_assert(offsetof(RegionSimulation, region) == 0x24, "RegionSimulation layout");
static_assert(offsetof(RegionSimulation, places) == 0x30, "RegionSimulation layout");
static_assert(offsetof(RegionSimulation, unblurredTree) == 0x58, "RegionSimulation layout");

static const char kSaveSignature[] = "#SqzSave v1.1 ASCII\n";    // the first line of a saved game (the pointer at 0x4ba368 points to it)
static Splash **const kSplash = (Splash **)0x4c75e8;
static float *const kLodClipDistance = (float *)0x4bb908;


static Inventory *InventoryOf(Possessions *p) { return p->inventory; }   // @00448990

static void Identity(float *m)
{
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// @00436370 reads the prop files into the second set of tables, or the third set when a second reader is given
static IntList *NewEmptyIntList() { return new IntList(); }

// @00416c60
PlaceDescriptor *PlaceDescriptor::Init()
{
    new (&function) String();
    position.x = position.y = position.z = 0;
    propPosition.x = propPosition.y = propPosition.z = 0;
    propScale.x = propScale.y = propScale.z = 0;
    links = NULL;
    destinations = NULL;
    prop = NULL;
    return this;
}

// @00416d40
void PlaceDescriptor::Destruct()
{
    if (destinations) {
        destinations->~IntList();
        Operator_Delete(destinations);
    }
    if (links) {
        links->~IntList();
        Operator_Delete(links);
    }
    if (prop) {
        CallVirtual<void>(prop, PROP_HIDE);
        CallVirtual<void>(prop, PROP_DESTROY, 1);
    }
    function.~String();
}

// @00416e50
RegionSimulation *RegionSimulation::Construct(int regionNumber, int locationNumber)
{
    LocationSimulation::Construct();
    ConstructPlaceTable(&places);
    vtbl = (void **)0x49d70c;
    region = regionNumber;
    location = locationNumber;
    return this;
}

// @00416ee0 (the table goes first, then the LocationSimulation part)
void RegionSimulation::Destruct()
{
    vtbl = (void **)0x49d70c;
    DestructPlaceTable(&places);
    LocationSimulation::Destruct();
}

// @00418a40
RegionSimulation *RegionSimulation::DestroyAndFree(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00416f50 Loads the region: the splash screen, the folder's region.ini, the scene, the props, the spots, the player and the
// camera. The progress bar of the splash screen is moved along the way (.35 .45 .55 .65 .75 .85 .9). Where the original leaves the
// rotation of the player's starting matrix as whatever is on the stack, ours uses the identity (SqueezilBody::Init only reads the
// position of it).
void RegionSimulation::Init()
{
    (*kSplash)->Load(2);
    LocationSimulation::Init();

    IniReader gameIni;
    *kLodClipDistance = gameIni.GetFloat("LODPropClipDistance", -1.0f);
    World *world = GetWorld();
    placeCount = 0;

    // <region folder>\region\region.ini
    char path[256];
    strcpy(path, GetLand()->GetPlaceName(region));
    strcat(path, "\\region\\region.ini");
    GameLog("NOTE: RegionSimulation::initialize: loading '%s'\n", path);

    IniReader regionIni(path);
    float clip = regionIni.GetFloat("LODPropClipDistance", -1.0f);
    if (*kLodClipDistance == -1.0f)
        *kLodClipDistance = clip;

    (*kSplash)->SetProgress(0.35f);
    ReadFunctionList(&regionIni);
    (*kSplash)->Step();
    LoadResources(&regionIni);
    (*kSplash)->SetProgress(0.45f);
    world->CreateScene(&regionIni, NULL);
    (*kSplash)->SetProgress(0.55f);
    WorldReadPropFiles(&regionIni, NULL);
    (*kSplash)->SetProgress(0.65f);

    regionIni.SetSection("Defaults");
    unblurredTree = FindPropTemplate(regionIni.GetString("UnblurredTreeSprite"), 0, 0);
    unblurredNutTree = FindPropTemplate(regionIni.GetString("UnblurredTreeSpriteNut"), 0, 0);
    blurredTree = FindPropTemplate(regionIni.GetString("BlurredTreeSprite"), 0, 0);
    blurredNutTree = FindPropTemplate(regionIni.GetString("BlurredTreeSpriteNut"), 0, 0);
    ReadLocations(&regionIni);
    (*kSplash)->SetProgress(0.75f);

    IniReader defaults;
    world->SetupLights(&defaults);
    world->SetupLights(&regionIni);

    // the player starts at the spot `location`
    Float3 start = GetPlace(location)->position;
    defaults.SetSection("Defaults");
    float matrix[16];
    Identity(matrix);
    matrix[12] = start.x;
    matrix[13] = start.y;
    matrix[14] = start.z;
    SqueezilBody *body = (SqueezilBody *)Operator_New(sizeof(SqueezilBody));
    if (body)
        body->Init(matrix, ReadBodyColor(&defaults));
    GameLog("Done with geometry.  Initializing soul: ");
    (*kSplash)->SetProgress(0.85f);

    Possessions *possessions = GetLand()->Lend(0);
    SqueezilRegionSoul *soul = (SqueezilRegionSoul *)Operator_New(0xe8);
    if (soul)
        soul->Init(body, possessions, this);
    (*kSplash)->SetProgress(0.9f);
    GameLog("OK.\nInitializing camera: ");
    AddCreature(0, body, soul);

    Identity(matrix);
    CameraBody *cameraBody = (CameraBody *)Operator_New(0xc0);
    if (cameraBody)
        cameraBody->Init(matrix);
    void *cameraSoul = Operator_New(0x168);                  // RegionCameraSoul
    cameraSoul = cameraSoul ? RegionCameraSoulInit(cameraSoul, cameraBody, body, soul) : NULL;
    AddCreature(1, cameraBody, cameraSoul);

    // the application sends the input to the two souls (the part of each that takes input events)
    char *app = (char *)GetApplication();
    *(void **)(app + 0x18) = soul ? (char *)soul + 0x80 : NULL;
    *(void **)(app + 0x1c) = cameraSoul ? (char *)cameraSoul + 4 : NULL;

    ((Panel *)GetPanel())->ShowQuest(GetRegion());
    *(int *)((char *)GetApplication() + 0xb4) = 0;                   // @0043aa20

    // the music
    regionIni.SetSection("Defaults");
    const char *midi = regionIni.GetString("MidiFile");
    if (strcmp(midi, "") != 0 && !StartMidi(midi))
        GameLog("Failure in midi manager initialization\n");

    (*kSplash)->Step();
    (*kSplash)->Hide();
    RunFrame();
    ResetClock();
    ((Panel *)GetPanel())->Update(1);

    // the script's StartFunction, called for the player
    ScriptCaller caller;
    caller.unknown0 = 0;
    caller.unknown4 = (int)soul;
    caller.soul = soul;
    caller.unknownC = 0;
    caller.actorIndex = ((LocationSimulation *)GetCurrentSimulation())->FindIdBySoul(soul);
    caller.prop = NULL;
    PostFunction("StartFunction", &caller, 0, 0);

    soul->StartFunction();
    if (*(int *)((char *)GetApplication() + 0x78))
        ApplicationSetInputMode(GetApplication(), 1);
    if (*(int *)0x4bd964 == 0)
        StartAudio();
    else
        *(int *)0x4bd964 = 0;
    FlushInputMessages(GetApplication());
    GameLog("OK.\n");
}

// @00417600
void RegionSimulation::Clear()
{
    while (AudioUpdate() > 0) {
    }
    UnloadSounds();
    if (*(int *)0x4bf458)
        StopMidi();
    ReleaseFunctionTable();         // @00422b80
    ReleaseFunctions();             // @00422b00
    UnloadResources();
    ClearWorldObjects();
    for (int i = 0; i < places.keys.count; i++) {
        PlaceDescriptor *place = places.values.At(i);
        if (place) {
            place->Destruct();
            Operator_Delete(place);
        }
    }
    ClearCreatures();
    LocationSimulation::Clear();
}

// @004189b0
Float3 *RegionSimulation::Velocity(Float3 *out, void *)
{
    out->x = out->y = out->z = 0;
    return out;
}

// @00417680 Reads every entry of [LocationList] (the keys are the spot numbers, the values the names of their sections), then
// drops the links to spots that do not exist -- or, as the original does it, the first n links of a spot with n bad ones (it
// removes by the position in the list of bad ones, not by the position of the bad link).
void RegionSimulation::ReadLocations(IniReader *ini)
{
    ini->SetSection("LocationList");
    StringList *keys = ini->GetKeys();
    for (int i = 0; i < keys->count; i++) {
        ini->SetSection("LocationList");
        String key = keys->At(i);
        const char *keyText = key.c_str();
        int id = atoi(keyText);
        const char *name = ini->GetString(keyText);
        GameLog("Parsing location %d, named %s: ", id, name);
        PlaceDescriptor *place = ReadLocation(ini, name, id);
        places.Insert(id, place);
    }
    if (keys) {
        keys->~StringList();
        Operator_Delete(keys);
    }

    for (int i = 0; i < places.keys.count; i++) {
        IntList *links = places.values.At(i)->links;
        IntList bad;
        for (int j = 0; j < links->count; j++) {
            if (!places.Contains(links->At(j))) {
                bad.Insert(j, -1);
                GameLog("Warning: place %d links to bad place %d!\n", places.keys.At(i), links->At(j));
            }
        }
        for (int k = bad.count - 1; k >= 0; k--)
            links->RemoveAt(k);
    }
}

// @004178e0
PlaceDescriptor *RegionSimulation::ReadLocation(IniReader *ini, const char *name, int id)
{
    PlaceDescriptor *place = new PlaceDescriptor;
    place->Init();
    place->id = id;

    ini->SetSection(name);
    String type(ini->GetString("Type"));
    const char *t = type.c_str();
    if (strcmp(t, "place") != 0 && strcmp(t, "portal") != 0 && strcmp(t, "navspot") != 0 && strcmp(t, "key") != 0 &&
        strcmp(t, "relic") != 0) {
        GameLog("error: type %s not recognized!\n", t);
        return place;
    }

    placeCount++;
    int incomplete = 0;
    if (strlen(ini->GetString("Location")) == 0) {
        place->position.x = place->position.y = place->position.z = 0;
        incomplete = 1;
    }
    ini->GetFloats("Location", &place->position.x);

    if (strlen(ini->GetString("PropPosition")) == 0) {
        place->propPosition.x = place->propPosition.y = place->propPosition.z = 0;
        incomplete = 1;
    }
    ini->GetFloats("PropPosition", &place->propPosition.x);

    if (strlen(ini->GetString("PropScale")) == 0) {
        place->propScale.x = place->propScale.y = place->propScale.z = 1.0f;
        incomplete = 1;
    }
    ini->GetFloats("PropScale", &place->propScale.x);

    bool isPlace = strcmp(t, "place") == 0;
    bool isPortal = strcmp(t, "portal") == 0;
    if (isPlace || isPortal) {
        if (strlen(ini->GetString("Destination")) == 0) {
            GameLog("error: no Destination entry!\n");
            place->destinations = NewEmptyIntList();
            incomplete = 1;
        } else {
            place->destinations = ini->GetIntList("Destination");
        }
    } else {
        place->destinations = NewEmptyIntList();
    }

    if (strlen(ini->GetString("Links")) == 0) {
        GameLog("error: no Links entry!\n");
        place->links = NewEmptyIntList();      // (replaced by the next line, as in the original, which leaks it)
        incomplete = 1;
    }
    place->links = ini->GetIntList("Links");

    const char *function = ini->GetString("Function");
    if (strlen(function) != 0)
        place->function = String(function);

    if (!incomplete)
        GameLog("data OK,\n  ");

    if (isPlace) {
        // a place is a tree on the map: a frame of its own where the prop stands
        float matrix[16];
        Identity(matrix);
        matrix[12] = place->propPosition.x;
        matrix[13] = place->propPosition.y;
        matrix[14] = place->propPosition.z;
        Scene *scene = GetScene();
        place->frame = scene->CreateSceneFrame();
        scene->SetFrameTransform(place->frame, matrix);
        RegisterPlace(place, id);
        place->kind = PlaceDescriptor::PLACE;
        return place;
    }

    if (strcmp(t, "navspot") == 0)
        place->kind = PlaceDescriptor::NAV_SPOT;
    else if (strcmp(t, "portal") == 0)
        place->kind = PlaceDescriptor::PORTAL;
    else
        place->kind = PlaceDescriptor::KEY_OR_RELIC;

    String propName(ini->GetString("Prop"));
    if (strlen(propName.c_str()) == 0) {
        GameLog("warning: no prop.\n");
        return place;
    }
    Prop *prop = LoadProp(ini, NULL, propName.c_str(), "Prop");
    if (prop)
        prop->VBuildFrame(GetScene()->GetSceneChild());
    else
        GameLog("ERROR: couldn't locate prop %s at location!\n", propName.c_str());
    return place;
}

// @004180e0
void RegionSimulation::RegisterPlace(PlaceDescriptor *place, int id)
{
    int state = LocationState(place, id);
    place->state = state;
    ShowPlaceProp(place, state);
}

// @004183e0
void RegionSimulation::ShowPlaceProp(PlaceDescriptor *place, int state)
{
    if (place->prop) {
        CallVirtual<void>(place->prop, PROP_HIDE);
        CallVirtual<void>(place->prop, PROP_DESTROY, 1);
    }
    Prop *sprite = NULL;
    switch (state) {
        case 0: sprite = blurredTree; break;
        case 1: sprite = unblurredTree; break;
        case 2: sprite = blurredNutTree; break;
        case 3: sprite = unblurredNutTree; break;
    }
    Float3 origin = {0, 0, 0}, scale = {1, 1, 1}, direction = {0, 0, 1}, up = {0, 1, 0};
    place->prop = sprite->Instantiate(NULL, origin, scale, direction, up);
    place->prop->VShow(place->frame);                       // slot 4
    place->prop->VApplyScale(place->frame, place->propScale);
    place->state = state;
}

// @00418590
int RegionSimulation::LocationState(PlaceDescriptor *place, int id)
{
    Possessions *possessions = GetLand()->Lend(0);
    Inventory *inventory = InventoryOf(possessions);
    int finished = 0;
    Array<int> *done = NULL;
    int count = InventoryCount(inventory, "PlacesCompleted");
    if (count > 0)
        done = InventoryItems(inventory, "PlacesCompleted");
    for (int i = 0; i < count && !finished; i += 2) {
        if (done->At(i) == region && done->At(i + 1) == id)
            finished = 1;
    }
    int nextTo = 0;
    for (int i = 0; i < place->links->count; i++)
        if (place->links->At(i) == location)
            nextTo = 1;
    if (finished)
        return nextTo ? 1 : 0;
    return nextTo ? 3 : 2;
}

// @00418170
void RegionSimulation::SetLocation(int newLocation)
{
    location = newLocation;
    GetPlace(newLocation);
    for (int i = 0; i < PlaceCount(); i++) {
        PlaceDescriptor *place = places.values.At(i);
        if (place->kind == PlaceDescriptor::PLACE) {
            int state = LocationState(place, place->id);
            if (place->state != state)
                ShowPlaceProp(place, state);
        }
    }

    Possessions *possessions = GetLand()->Lend(0);
    Float3 faraway = {0, -100000.0f, 0};          // the sound comes from nowhere in particular
    if (location >= 300 && location < 400) {
        int piece = location % 300 - 1;
        Inventory *inventory = InventoryOf(possessions);
        if (InventoryKeyFlags(inventory)[region * 3 + piece] == 0) {
            PlayEffect("keyEffect", faraway);
            InventoryKeyFlags(InventoryOf(possessions))[region * 3 + piece] = 1;
            ((Panel *)GetPanel())->Invalidate();
        }
    }
    if (location >= 400 && location < 500) {
        if (KeysFound(InventoryOf(possessions), region) && region != 1) {
            PlayEffect("relicEffect", faraway);
            InventoryRelicFlags(InventoryOf(possessions))[region] = 1;
            ((Panel *)GetPanel())->UpdateRegionSprites();
        }
    }
}

// @00418680 Loads a saved game: the file holds a signature line, "region location", and the player's inventory. The player's
// possessions get the new inventory, and a new RegionSimulation for the saved region takes over from this one (this one is
// destroyed). Returns 0 when the file is not a save.
int RegionSimulation::Load(const char *path)
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

    void *playerSoul = DynamicCast(((LocationSimulation *)GetCurrentSimulation())->GetSoul(0), kSoulType, kRegionSoulType);
    Possessions *possessions = CallVirtual<Possessions *>(playerSoul, SQS_TAKE_POSSESSIONS);
    PossessionsSetInventory(possessions, inventory);
    GetLand()->Return(0, possessions);

    void *current = GetCurrentSimulation();
    CallVirtual<void>(current, 2);                                   // Clear
    current = GetCurrentSimulation();
    if (current)
        CallVirtual<void>(current, 0, 1);                           // destroyed (this one)
    RegionSimulation *next = (RegionSimulation *)Operator_New(0x68);
    if (next)
        next->Construct(newRegion, newLocation);
    SetCurrentSimulation(next);
    CallVirtual<void>(next, 1);                                      // Init
    ((Panel *)GetPanel())->UpdateRegionSprites();
    SetGamePaused(1);
    return 1;
}

// @004188c0 Writes a saved game (see Load). Returns 0 when the file can't be written.
int RegionSimulation::Save(const char *path)
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
    void *playerSoul = DynamicCast(((LocationSimulation *)GetCurrentSimulation())->GetSoul(0), kSoulType, kRegionSoulType);
    Possessions *possessions = CallVirtual<Possessions *>(playerSoul, SQS_GET_POSSESSIONS);
    if (!InventorySave(InventoryOf(possessions), file)) {
        GameFclose(file);
        return 0;
    }
    GameFclose(file);
    return 1;
}
