// SqueezilRegionSoul: see SqueezilRegionSoul.h. Reconstructed from SQZ.EXE 0x436710-0x438380.
#include "SqueezilRegionSoul.h"
#include "SqueezilSoulInternal.h"
#include "RegionSimulation.h"
#include "PlaceSimulation.h"
#include "LandSimulation.h"
#include "SqueezilBody.h"
#include "Scene.h"
#include "IniReader.h"
#include "Script.h"
#include "FrameProp.h"
#include "Original.h"
#include "VirtualCall.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

static_assert(offsetof(SqueezilRegionSoul, region) == 0x88, "SqueezilRegionSoul layout");
static_assert(offsetof(SqueezilRegionSoul, turnRate) == 0xb0, "SqueezilRegionSoul layout");

// The settings read once from the game's ini file (the first soul made reads them)
static int   &gSettingsRead     = *(int *)0x4bc5e4;
static int   &gDisplayBridge    = *(int *)0x4c7334;     // DisplayRegionBridge: show the strip towards the aimed spot
static int   &gFlightDuration   = *(int *)0x4c7328;     // RegionFlightDuration: frames of a flight (30)
static float &gTurnScale        = *(float *)0x4c733c;   // TurnRateScale: degrees per frame of turn rate for a full push (0.02)
static float &gTurnDamping      = *(float *)0x4c7330;   // TurnRateDamping: what is left of the turn rate after a frame (0.1)
static float &gArcHeightRatio   = *(float *)0x4c7344;   // RegionFlightArcHeightRatio: the arc's height over the distance (0.1)
static float &gArcTilt          = *(float *)0x4c732c;   // RegionFlightArcInitialTilt: how far the flight starts upwards (0.3)
static float &gBridgeWidth      = *(float *)0x4c7338;   // RegionBridgeWidth: half the strip's width (0.05)
static IDirect3DRMFrame *&gBridgeFrame = *(IDirect3DRMFrame **)0x4c7340;   // the strip being shown
static DWORD &gBlinkLast        = *(DWORD *)0x4bc5e8;   // when a tree last blinked
static DWORD &gBlinkNow         = *(DWORD *)0x4bc5ec;

static const float kTwoPi = 6.2831855f;                 // 0x49e0c0
static const float kPointedAt = 0.349066f;              // 20 degrees (0x3eb2b8c2)

// @00436710
SqueezilRegionSoul *SqueezilRegionSoul::Init(SqueezilBody *bodyArg, Possessions *possessionsArg, RegionSimulation *regionArg)
{
    SqueezilSoul::Init(bodyArg, possessionsArg, NULL);
    vtbl = (void **)0x49e12c;
    region = regionArg;
    inputVtbl = (void **)0x49d2c4;
    grantVtbl = (void **)0x49e10c;
    inputVtbl = (void **)0x49e0e4;

    if (!gSettingsRead) {
        IniReader ini;
        gDisplayBridge = ini.GetInt("DisplayRegionBridge", 0);
        gFlightDuration = ini.GetInt("RegionFlightDuration", 30);
        gTurnScale = ini.GetFloat("TurnRateScale", 0.02f);
        gTurnDamping = ini.GetFloat("TurnRateDamping", 0.1f);
        gArcHeightRatio = ini.GetFloat("RegionFlightArcHeightRatio", 0.1f);
        gArcTilt = ini.GetFloat("RegionFlightArcInitialTilt", 0.3f);
        gBridgeWidth = ini.GetFloat("RegionBridgeWidth", 0.05f);
        gSettingsRead = 1;
    }

    location = region->GetLocation();
    target = 0;
    float matrix[16];
    memset(matrix, 0, sizeof matrix);
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
    SetTranslation(matrix, ToVector(region->GetPlace(location)->position));
    body->SetMatrix(matrix);
    flightFrame = 0;
    SetState(SS_STAND, 0);
    typing = 0;
    turnRate = 0;
    unknownE4 = -1;
    flying = 0;

    // the first visit to a spot is remembered: "PlacesBeenTo" holds (region, spot) pairs
    if (!BeenTo(location)) {
        Inventory *inventory = GetLand()->Lend(0)->inventory;
        InventoryAdd(inventory, "PlacesBeenTo", region->GetRegion());
        InventoryAdd(inventory, "PlacesBeenTo", region->GetLocation());
    }
    return this;
}

// @004369c0 slot 0
SqueezilRegionSoul *SqueezilRegionSoul::DestroyAndFree(int flags)
{
    SqueezilSoul::Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00436a60 The function the spot is named after runs (with the soul as its caller)
void SqueezilRegionSoul::StartFunction()
{
    PlaceDescriptor *place = region->GetPlace(location);
    if (strlen(place->function.c_str()) != 0) {
        ScriptCaller caller;
        caller.unknown0 = 0;
        caller.unknown4 = (int)this;
        caller.soul = this;
        caller.unknownC = 0;
        caller.actorIndex = 0;
        caller.prop = NULL;          // (the original leaves this field as it was: the spot's prop was meant)
        PostFunction(place->function.c_str(), &caller, 0, 0);
    }
}

// @00436af0 The action button: from a place, go into it; from a portal, through it (to another region). Not while a flight is on
// (`typing` is 0 then), and only where the spot leads somewhere and the player has earned the way.
void SqueezilRegionSoul::Enter()
{
    SqueezilRegionSoul *soul = this;    // (the hook gives the soul; the original is called on the part at +0x80)
    if (soul->flying)
        return;
    PlaceDescriptor *here = soul->region->GetPlace(soul->region->GetLocation());
    IntList *destinations = here->destinations;
    if (destinations->count == 0)
        return;
    if (here->kind == PlaceDescriptor::PORTAL) {
        Inventory *inventory = GetLand()->Lend(0)->inventory;
        int regionNumber = soul->region->GetRegion();
        if (regionNumber == 1) {
            int to = destinations->At(0);
            if (to == 6) {
                if (!InventoryAllRelics(inventory))
                    return;
            } else if (inventory ? RelicFlag(inventory, to) != 0 : 0) {
                return;
            }
        } else if (regionNumber == 6 && destinations->At(0) == 7) {
            if (!KeysFound(inventory, regionNumber))
                return;
        } else {
            if (regionNumber == 7)
                return;
            if (RelicFlag(inventory, regionNumber) == 0)
                return;
        }
    }

    Float3 faraway = {0, -100000.0f, 0};
    PlayEffect("enterEffect", faraway);
    if (destinations->count > 1) {
        // a portal to another region: (region, spot)
        GetLand()->Return(0, CallVirtual<Possessions *>(soul, SQS_TAKE_POSSESSIONS));
        int nextRegion = destinations->At(0);
        int nextLocation = destinations->At(1);
        void *current = GetCurrentSimulation();
        CallVirtual<void>(current, 2);                                  // Clear
        current = GetCurrentSimulation();
        if (current)
            CallVirtual<void>(current, 0, 1);                           // destroy it
        RegionSimulation *next = (RegionSimulation *)Operator_New(0x68);
        if (next)
            next->Construct(nextRegion, nextLocation);
        SetCurrentSimulation(next);
        CallVirtual<void>(next, 1);                                     // Init
    } else {
        // a place: save the game first, then leave the region for it
        char path[112];
        strcpy(path, (const char *)0x4c6400);
        strcat(path, "Autosave");
        CallVirtual<int>(GetCurrentSimulation(), 11, path);           // Save
        GetLand()->Return(0, CallVirtual<Possessions *>(soul, SQS_TAKE_POSSESSIONS));
        int regionNumber = soul->region->GetRegion();
        int placeNumber = destinations->At(0);
        int locationNumber = soul->region->GetLocation();
        void *current = GetCurrentSimulation();
        CallVirtual<void>(current, 2);
        current = GetCurrentSimulation();
        if (current)
            CallVirtual<void>(current, 0, 1);
        PlaceSimulation *next = (PlaceSimulation *)Operator_New(0x44);
        if (next)
            next->Construct(regionNumber, placeNumber, locationNumber);
        SetCurrentSimulation(next);
        CallVirtual<void>(next, 1);
    }
}

// @00436e10 (this = the soul). Typing a spot number (what 0x18) moves the squeezil to that spot at once, when it is on the ground.
void SqueezilRegionSoul::OnEvent(int what, const char *text)
{
    if (what != 0x18)
        return;
    typing = 0;
    strcpy(typed, text);
    typedLength = (int)strlen(typed);
    if (!GetBody()->IsOnGround())
        return;
    int spot = atoi(typed);
    if (!region->HasPlace(spot))
        return;
    location = spot;
    float matrix[16];
    memcpy(matrix, GetBody()->matrix, sizeof matrix);
    SetTranslation(matrix, ToVector(region->GetPlace(location)->position));
    GetBody()->SetMatrix(matrix);
    region->SetLocation(location);
}

// @00436fc0 A flat strip of width 2 * RegionBridgeWidth from one spot to the other, lying across the line between them (a coloured
// quad in a frame of its own, made by Scene::CreateColoredQuad). Returns the frame.
IDirect3DRMFrame *SqueezilRegionSoul::BuildBridge(Float3 fromPoint, Float3 toPoint, Float3 color)
{
    Vector3f a = ToVector(fromPoint), b = ToVector(toPoint);
    Vector3f delta = b - a;
    Vector3f up(0, 1, 0);
    Vector3f side = Cross(delta, up);
    Normalize(side);
    side = side * gBridgeWidth;
    Vector3f p0 = a - side, p1 = a + side, p2 = b + side, p3 = b - side;
    Vector3f normal = Cross(side, delta);
    Normalize(normal);
    return GetScene()->CreateColoredQuad(p0.x, p0.y, p0.z, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z,
                                         normal.x, normal.y, normal.z, color.x, color.y, color.z);
}

// @00438110 The strip towards the aimed spot is replaced by the one towards the new one (target 0: just taken away).
void SqueezilRegionSoul::ShowBridge(int newTarget)
{
    Scene *scene = GetScene();
    if (newTarget == 0) {
        if (target != 0)
            DeleteChildFromFrame(scene->GetSceneChild(), gBridgeFrame);
        return;
    }
    if (target != 0)
        DeleteChildFromFrame(scene->GetSceneChild(), gBridgeFrame);
    Float3 start = region->GetPlace(location)->position;
    Float3 end = region->GetPlace(newTarget)->position;
    Float3 white = {1, 1, 1};
    gBridgeFrame = BuildBridge(start, end, white);
    AddChildToFrame(scene->GetSceneChild(), gBridgeFrame);
    gBridgeFrame->Release();
}

// @00437f40 Whether the player has been to the spot (the inventory's "PlacesBeenTo" holds (region, spot) pairs)
int SqueezilRegionSoul::BeenTo(int place)
{
    Inventory *inventory = GetLand()->Lend(0)->inventory;
    int count = InventoryCount(inventory, "PlacesBeenTo");
    int found = 0;
    if (count > 0) {
        Array<int> *been = InventoryItems(GetLand()->Lend(0)->inventory, "PlacesBeenTo");
        for (int i = 0; i < count; i += 2) {
            if (region->GetRegion() == been->At(i) && been->At(i + 1) == place) {
                found = 1;
                break;
            }
        }
    }
    return found;
}

// @00437e40 Whether the player has earned the way over a portal: into region 6 once all the relics are in, and out of a region
// once its own relic is; region 6 leads on to 7 when its three key pieces are in.
int SqueezilRegionSoul::LinkOpen(PlaceDescriptor *here, PlaceDescriptor *)
{
    if (here->kind != PlaceDescriptor::PORTAL)
        return 0;
    Inventory *inventory = GetLand()->Lend(0)->inventory;
    int regionNumber = region->GetRegion();
    if (regionNumber == 1) {
        if (here->destinations->At(0) == 6)
            return InventoryAllRelics(inventory) != 0;
        return 1;
    }
    if (regionNumber != 6 && regionNumber != 7)
        return RelicFlag(inventory, regionNumber) != 0;
    if (regionNumber == 6) {
        if (here->destinations->At(0) != 7)
            return 0;
        return KeysFound(inventory, regionNumber) != 0;
    }
    return 0;
}

// @00438020 The aimed spot's tree blinks (every 400 ms it switches between its two looks); the one aimed at before goes back to
// the steady look.
void SqueezilRegionSoul::BlinkSelection(int selected)
{
    if (GetBody()->IsOnGround() && selected != 0) {
        PlaceDescriptor *place = region->GetPlace(selected);
        if (place->kind == PlaceDescriptor::PLACE) {
            gBlinkNow = timeGetTime();
            if (gBlinkNow - gBlinkLast >= 400) {
                int look = place->state <= 3 ? (place->state ^ 1) : 1;
                region->ShowPlaceProp(place, look);
                gBlinkLast = gBlinkNow;
            }
        }
    }
    if (target != 0 && selected != target) {
        PlaceDescriptor *old = region->GetPlace(target);
        if (old->kind == PlaceDescriptor::PLACE) {
            if (old->state == 2)
                region->ShowPlaceProp(old, 3);
            else if (old->state == 0)
                region->ShowPlaceProp(old, 1);
        }
    }
}

// @004373d0 slot 5: one frame on the map.
void SqueezilRegionSoul::UpdateRegion()
{
    float best = kPointedAt;
    int selected = 0;
    SqueezilBody *theBody = GetBody();
    float *matrix = theBody->matrix;
    float heading = atan2f(matrix[8], matrix[10]);
    Vector3f pos(matrix[12], matrix[13], matrix[14]);

    PlaceDescriptor *here = region->GetPlace(location);
    int hereDone = here->kind == PlaceDescriptor::PLACE && GetLand()->PlaceCompleted(region->GetRegion(), here->id);

    for (int i = 0; i < here->links->count; i++) {
        PlaceDescriptor *link = region->GetPlace(here->links->At(i));
        int linkDone = link->kind == PlaceDescriptor::PLACE && GetLand()->PlaceCompleted(region->GetRegion(), link->id);
        bool aimable = false;
        if (here->kind == PlaceDescriptor::PORTAL || here->kind == PlaceDescriptor::NAV_SPOT || hereDone || linkDone)
            aimable = true;
        else if (link->kind != PlaceDescriptor::PORTAL && BeenTo(link->id))
            aimable = true;
        else if (here->id >= 300 && here->id < 400)
            aimable = true;
        else if (link->id >= 300 && link->id < 400)
            aimable = true;
        else if (LinkOpen(here, link))
            aimable = true;
        else if (here->id >= 400 && here->id < 500 && link->kind != PlaceDescriptor::PORTAL)
            aimable = true;
        if (!aimable)
            continue;

        float angle = fabsf(atan2f(link->position.x - pos.x, link->position.z - pos.z) - heading);
        while (angle > kTwoPi)
            angle -= kTwoPi;
        if (angle < best) {
            best = angle;
            selected = here->links->At(i);
        }
    }

    if (theBody->IsOnGround() && target != selected && gDisplayBridge)
        ShowBridge(selected);
    BlinkSelection(selected);
    target = selected;

    // pushing forward while aiming at a spot starts the flight
    if (theBody->IsOnGround() && target != 0 && ApplicationForwardAxis(GetApplication()) > 0.0f) {
        from = ToVector(region->GetPlace(location)->position);
        to = ToVector(region->GetPlace(target)->position);
        CallVirtual<void>(this, SQS_SET_STATE, SS_FLY, 0);
        flightFrame = 0;
    }

    if (theBody->IsOnGround()) {
        // turning on the spot
        turnRate = ApplicationTurnAxis(GetApplication()) * gTurnScale + turnRate;
        float rotation[16], turned[16];
        RotationY(rotation, turnRate * kRadiansPerDegree);
        MultiplyMatrix(turned, rotation, CallVirtual<float *>(theBody, BODY_GET_MATRIX));
        theBody->SetMatrix(turned);
        turnRate *= gTurnDamping;
        return;
    }

    // flying
    flightFrame++;
    if (flightFrame >= gFlightDuration) {
        // arrived
        location = target;
        CallVirtual<void>(this, SQS_SET_STATE, SS_LAND, 0);
        CallVirtual<void>(this, SQS_SET_STATE, SS_STAND, 0);
        flying = 0;
        const float *m = CallVirtual<float *>(theBody, BODY_GET_MATRIX);
        Vector3f forward(m[8], 0, m[10]);
        Normalize(forward);
        Vector3f upward(0, 1, 0);
        Vector3f right = Cross(upward, forward);
        Vector3f where = ToVector(region->GetPlace(location)->position);
        float placed[16];
        SetRows(placed, right, upward, forward, where);
        theBody->SetMatrix(placed);
        region->SetLocation(location);
        StartFunction();
        if (!BeenTo(location)) {
            Inventory *inventory = GetLand()->Lend(0)->inventory;
            InventoryAdd(inventory, "PlacesBeenTo", region->GetRegion());
            InventoryAdd(inventory, "PlacesBeenTo", region->GetLocation());
        }
        return;
    }

    // a step of the arc: along the line from `from` to `to`, up and down over a parabola, tilted along it
    flying = 1;
    Vector3f delta = to - from;
    float t = (float)flightFrame / (float)gFlightDuration;
    Vector3f p = from + delta * t;
    Vector3f forward = delta;
    Normalize(forward);
    float rise = gArcTilt * -2.0f;
    forward.y = forward.y + rise * t + gArcTilt;       // pitched up at the start and down at the end
    float u = fabsf(1.0f - 2.0f * t);
    float height = Length(delta) * gArcHeightRatio;
    p.y = (p.y - u * u * height) + height;
    Vector3f upward(0, 1, 0);
    Vector3f right = Cross(upward, forward);
    Vector3f up2 = Cross(forward, right);
    float placed[16];
    SetRows(placed, right, up2, forward, p);
    OrthonormalizeMatrix(placed);
    theBody->SetMatrix(placed);
}
