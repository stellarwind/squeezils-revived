// SqueezilPlayerSoul: see SqueezilPlayerSoul.h. Reconstructed from SQZ.EXE 0x41ee30-0x41fb80.
#include "SqueezilPlayerSoul.h"
#include "SqueezilSoulInternal.h"
#include "PlaceSimulation.h"
#include "RegionSimulation.h"
#include "LandSimulation.h"
#include "SqueezilBody.h"
#include "IniReader.h"
#include "Script.h"
#include "FrameProp.h"
#include "Original.h"
#include "VirtualCall.h"
#include <math.h>

static_assert(offsetof(SqueezilPlayerSoul, attackHeld) == 0x84, "SqueezilPlayerSoul layout");
static_assert(offsetof(SqueezilPlayerSoul, enoughNuts) == 0x98, "SqueezilPlayerSoul layout");

static const void *const kPlayerSoulVtable      = (void *)0x49d91c;
static const void *const kPlayerGrantVtable     = (void *)0x49d8fc;
static const void *const kPlayerInputVtable     = (void *)0x49d8d4;
static const void *const kInputTargetVtable     = (void *)0x49d2c4;   // the base class's, until the constructor has finished

// The settings read once from the game's ini file (the first soul made reads them)
static int   &gSettingsRead     = *(int *)0x4bad00;
static float &gSleepTime        = *(float *)0x4c7058;   // PlayerSleepTime: seconds standing still before the squeezil lies down (15)
static int   &gFoodNutsToFinish = *(int *)0x4bad04;     // FoodNutsToFinish: nuts carried that run EnoughNutsFunction (15)
static SqueezilPlayerSoul *&gThePlayer = *(SqueezilPlayerSoul **)0x4bad08;   // the first player soul made, until it is destroyed

static const float kNoPush = 0.01f;     // 0x49d8a8: less than this and a forward push counts as none
static const float kNoTurn = 0.1f;      // 0x49d8b0: and a turn
static const float kNoPitch = 0.1f;     // @0x3dcccccd in the air input

// @0041ee30
SqueezilPlayerSoul *SqueezilPlayerSoul::Init(SqueezilBody *bodyArg, Possessions *possessionsArg, void *placeArg)
{
    SqueezilSoul::Init(bodyArg, possessionsArg, placeArg);
    attackHeld = 0;
    unknown90 = 0;
    enoughNuts = 0;
    sleepTimer = 0.0f;
    inputVtbl = (void **)kInputTargetVtable;
    unknown88 = -1;
    wasOnGround = 1;
    grantVtbl = (void **)kPlayerGrantVtable;
    inputVtbl = (void **)kPlayerInputVtable;
    vtbl = (void **)kPlayerSoulVtable;

    IniReader ini;
    if (!gSettingsRead) {
        gSleepTime = ini.GetFloat("PlayerSleepTime", 15.0f);
        gSettingsRead = 1;
    }
    gFoodNutsToFinish = ini.GetInt("FoodNutsToFinish", 15);
    if (!gThePlayer)
        gThePlayer = this;
    return this;
}

// @0041ef80 (the part that is not the base class's)
void SqueezilPlayerSoul::Destruct()
{
    vtbl = (void **)kPlayerSoulVtable;
    grantVtbl = (void **)kPlayerGrantVtable;
    inputVtbl = (void **)kPlayerInputVtable;
    gThePlayer = NULL;
    SqueezilSoul::Destruct();
}

// @0041ef60 slot 0
SqueezilPlayerSoul *SqueezilPlayerSoul::DestroyAndFree(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @0041fab0 slot 9
void SqueezilPlayerSoul::Hit(int knock, int sprayNuts, int unused)
{
    if (*kLoopingSoundPlaying)
        StopLoopingSounds();
    SqueezilSoul::Hit(knock, sprayNuts, unused);
}

// @0041f1c0 slot 39 What the player wishes on the ground: how far to push forward and turn (the axes, or a one-press tap when the
// axis is idle), and which state that means: nothing, walk, run, a skid when pushing back while still going forward, or the attack
// while the attack button is held. `c` also gets the motion settings of the current state (GetGroundMotion).
void SqueezilPlayerSoul::ReadGroundInput(GroundControl *c, int *next)
{
    void *application = GetApplication();
    attackHeld = InputHeldAttack();
    int run = InputHeldRun();
    float turn = ApplicationTurnAxis(application);
    float forward = ApplicationForwardAxis(application);
    SqueezilBody *theBody = SoulCall::Body(this);
    float speed = BodyCall::Speed(theBody);
    int current = SoulCall::GetState(this);

    if (frozen) {
        c->ground = physics->walk;
        c->forward = 0.0f;
        c->turn = 0.0f;
        *next = SS_STAND;
        return;
    }

    float forwardTap = ApplicationForwardTap(application);
    float turnTap = ApplicationTurnTap(application);
    if (forwardTap != 0.0f && fabsf(forward) <= kNoPush)
        forward = physics->fwdTapFactor * forwardTap;
    if (turnTap != 0.0f && fabsf(turn) <= kNoTurn)
        turn = physics->rotTapFactor * turnTap;

    int wish;
    if (fabsf(forward) > kNoPush) {
        if (attackHeld) {
            wish = SS_ATTACK;
        } else {
            wish = run ? SS_RUN : SS_WALK;
            if (forward < 0.0f && speed > 0.01f) {
                // pushing back while still moving forward along the heading: brake
                Float3 *direction = CallVirtual<Float3 *>(SoulCall::Body(this), BODY_GET_FACING);
                float *matrix = BodyCall::Matrix(SoulCall::Body(this));
                float along = direction->z * matrix[10] + direction->y * matrix[9] + matrix[8] * direction->x;
                if (along > 0.0f)
                    wish = SS_SKID;
            }
        }
        c->forward = forward;
    } else {
        wish = SS_STAND;
        c->forward = 0.0f;
    }
    *next = (wish == current) ? SS_NONE : wish;
    CallVirtual<void>(this, SQS_GET_GROUND_MOTION, current, c);
    c->turn = (fabsf(turn) > kNoTurn && current != SS_SLIP) ? turn : 0.0f;
}

// @0041f3e0 slot 40 What the player wishes in the air: the pitch (kNoInput for none) and the turn, and whether to dive (the attack
// button pressed in the air) or to go back to gliding (it was let go). `c` also gets the flight settings of the current state.
void SqueezilPlayerSoul::ReadAirInput(AirControl *c, int *next)
{
    void *application = GetApplication();
    float push = ApplicationForwardAxis(application);
    float sign = (push < 0.0f) ? -1.0f : 1.0f;
    float pitchSize = push * sign;
    int attack = InputHeldAttack();
    float turn = ApplicationTurnAxis(application);
    float forwardTap = ApplicationForwardTap(application);
    float turnTap = ApplicationTurnTap(application);
    *next = SS_NONE;

    if (frozen) {
        c->pitch = kNoInput;
        c->turn = 0.0f;
        c->flight = physics->fly;
        return;
    }

    if (forwardTap != 0.0f && pitchSize < kNoPitch) {
        pitchSize = physics->fwdTapFactor;      // (the original reuses the variable for the factor, so the test below sees it)
        push = pitchSize * forwardTap;
    }
    if (turnTap != 0.0f && fabsf(turn) <= kNoTurn)
        turn = physics->rotTapFactor * turnTap;

    int current = SoulCall::GetState(this);
    SoulCall::GetAirMotion(this, current, c);

    if (attack) {
        if (!attackHeld && current != SS_DIVE)
            *next = SS_DIVE;
    } else {
        attackHeld = 0;
        if (current == SS_DIVE)
            *next = SS_FLY;
    }

    c->pitch = (pitchSize > kNoPitch) ? push : kNoInput;
    c->turn = (fabsf(turn) > kNoTurn) ? turn : 0.0f;
}

// @0041f5a0 slot 5 One frame: the base class's, then sleep and the landing sound
void SqueezilPlayerSoul::UpdatePlayer()
{
    float pitch, yawRate;
    BodyCall::GetAngles(SoulCall::Body(this), &pitch, &yawRate, &pitch);
    if (finished)
        return;

    int before = SoulCall::GetState(this);
    SqueezilSoul::Update();
    int after = SoulCall::GetState(this);
    if (before == 0 && after == 0 && fabsf(yawRate) <= kNoTurn && frozen == 0) {
        // standing still: after PlayerSleepTime seconds the squeezil lies down
        sleepTimer += FrameSeconds();
        if (!(sleepTimer <= gSleepTime)) {
            SoulCall::SetState(this, SS_LIE_DOWN, 0);
            PlayLoopingSound("sleepEffect");
            StartAudio();
        }
    } else {
        sleepTimer = 0.0f;
    }

    int onGround = BodyCall::IsOnGround(SoulCall::Body(this));
    if (onGround != wasOnGround) {
        if (BodyCall::IsOnGround(SoulCall::Body(this)) == 1)
            PlayEffect("landEffect", ToFloat3(BodyCall::Position(SoulCall::Body(this))));
        wasOnGround = BodyCall::IsOnGround(SoulCall::Body(this));
    }
}

// @0041f750 The nuts carried are enough to finish the level: run EnoughNutsFunction (once)
int SqueezilPlayerSoul::PostUpdate()
{
    if (enoughNuts)
        return 1;
    if (InventoryCount(SoulCall::GetInventory(this), "FoodNutGoodie") < gFoodNutsToFinish)
        return 0;
    ScriptCaller caller;
    int id = ((LocationSimulation *)GetCurrentSimulation())->FindIdBySoul(this);
    ScriptCallerInitSoul(&caller, this, id, NULL);
    PostFunction("EnoughNutsFunction", &caller, 0, 0);
    enoughNuts = 1;
    return 1;
}

// @0041f7f0 Called when the level is over. Dead (`finished`): the player's possessions go back to the land, the creatures are cleared
// and the place starts over. Otherwise the level is won: the nuts carried are banked under TotalFoodNuts, the place is marked
// completed (once: GoldPlacesCompleted or DiamondPlacesCompleted by the level's type, and the pair (region, spot) under
// PlacesCompleted), and the game goes back to the region map at the spot the player entered from.
void SqueezilPlayerSoul::Finish()
{
    PlaceSimulation *thePlace = (PlaceSimulation *)place;
    if (finished) {
        GetLand()->Return(0, CallVirtual<Possessions *>(this, SQS_TAKE_POSSESSIONS));
        CallVirtual<void>(thePlace, 7);                         // ClearCreatures
        thePlace->Start();
        return;
    }

    if (*kLoopingSoundPlaying)
        StopLoopingSounds();
    PlayEffect("levelEndEffect", ToFloat3(BodyCall::Position(SoulCall::Body(this))));

    Inventory *inventory = SoulCall::GetInventory(this);
    int carried = InventoryCount(inventory, "FoodNutGoodie");
    int banked = InventoryCount(inventory, "TotalFoodNuts");
    for (int i = 0; i < carried; i++)
        InventoryAdd(inventory, "TotalFoodNuts", banked + i);
    InventoryRemoveNuts(inventory);

    thePlace = (PlaceSimulation *)place;
    if (!GetLand()->PlaceCompleted(thePlace->region, thePlace->location)) {
        inventory = SoulCall::GetInventory(this);
        switch (thePlace->levelType) {
        case 0:
            InventoryAdd(inventory, "GoldPlacesCompleted", InventoryCount(inventory, "GoldPlacesCompleted") + 1);
            break;
        case 1:
            InventoryAdd(inventory, "DiamondPlacesCompleted", InventoryCount(inventory, "DiamondPlacesCompleted") + 1);
            break;
        }
        // (the original also counts the PlacesCompleted entries here and drops the number)
        InventoryAdd(inventory, "PlacesCompleted", thePlace->region);
        InventoryAdd(inventory, "PlacesCompleted", thePlace->location);
    }

    thePlace = (PlaceSimulation *)place;
    int regionNumber = thePlace->region;
    int locationNumber = thePlace->location;
    GetLand()->Return(0, CallVirtual<Possessions *>(this, SQS_TAKE_POSSESSIONS));
    void *current = GetCurrentSimulation();
    CallVirtual<void>(current, 2);                              // Clear
    current = GetCurrentSimulation();
    if (current)
        CallVirtual<void>(current, 0, 1);                       // destroy it
    RegionSimulation *next = (RegionSimulation *)Operator_New(0x68);
    if (next)
        next->Construct(regionNumber, locationNumber);
    SetCurrentSimulation(next);
    CallVirtual<void>(next, 1);                                 // Init
}

// @0041eff0 (this = the soul). Keys: 0x16 leave (only once the dying is over: starts the place again), 0x1e back to the level's
// start, 0x1f freeze, or unfreeze if it was frozen.
void SqueezilPlayerSoul::OnEvent(int what, const char *)
{
    if (what == 0x16) {
        if (finished)
            Finish();
    } else if (what == 0x1e) {
        PlaceLevel *level = ((PlaceSimulation *)place)->GetLevel();
        CallVirtual<void>(SoulCall::Body(this), BODY_PLACE_ON_FACE, &level->start.contact);
    } else if (what == 0x1f) {
        if (frozen == 0)
            CallVirtual<void>(this, SQS_FREEZE);
        else
            CallVirtual<void>(this, SQS_UNFREEZE);
    }
}

// @0041f070 (this = the soul) The jump button went down. Standing: crouch for a hop (the strength is set when it is let go);
// walking, slipping or attacking: crouch for a hop at full strength; running or scampering: jump; in the air after a jump or a hop:
// glide.
void SqueezilPlayerSoul::JumpPressed()
{
    if (frozen || !physics->unknown288)
        return;
    switch (SoulCall::GetState(this)) {
    case SS_STAND:
        SoulCall::SetState(this, SS_HOP_START, 0);
        BodyCall::SetAngles(SoulCall::Body(this), 0.0f, 0.0f, 0.0f);
        hopStrength = -1.0f;
        break;
    case SS_WALK:
    case SS_SLIP:
    case SS_ATTACK:
        SoulCall::SetState(this, SS_HOP_START, 0);
        hopStrength = 1.0f;
        break;
    case SS_RUN:
    case SS_SCAMPER:
    case SS_SCAMPER_STOP:
        SoulCall::StartJump(this, physics->jump.upSpeed, physics->jump.fwdSpeed, physics->jump.upDistance);
        SoulCall::SetState(this, SS_JUMP, 0);
        break;
    case SS_JUMP:
    case SS_HOP:
    case SS_SKIHOP:
        SoulCall::SetState(this, SS_FLY, 0);
        break;
    }
}

// @0041f160 (this = the soul) The jump button was let go: how long it was held sets the hop's strength
void SqueezilPlayerSoul::JumpReleased(float strength)
{
    if (!frozen && physics->unknown288)
        hopStrength = strength;
}

// @0041f180 (this = the soul) The spit button went down: the state spitting leads to
void SqueezilPlayerSoul::SpitPressed()
{
    int state = CallVirtual<int>(this, SQS_SPIT);
    SoulCall::SetState(this, state, 0);
}

// @0041f1a0 (this = the soul)
void SqueezilPlayerSoul::SpitReleased(float strength)
{
    CallVirtual<void>(this, SQS_SET_SPIT_STRENGTH, strength);
}
