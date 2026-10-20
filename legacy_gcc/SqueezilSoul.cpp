// SqueezilSoul: see SqueezilSoul.h. This file: the settings, the constructor and destructor, the state machine, the goodies and
// the surface type, and Update (SQZ.EXE 0x44f1a0-0x450e90, 0x451270-0x451630, 0x4569c0-0x457040 and the small functions at
// 0x404fc0-0x405030). The movement code is in SqueezilSoulMove.cpp.
#include <math.h>
#include <string.h>
#include <new>
#include "SqueezilSoulInternal.h"
#include "IniReader.h"
#include "Container.h"
#include "Simulation.h"
#include "Script.h"
#include "FrameProp.h"

// The shared defaults and the settings outside the block. They stay at their original addresses while original code (the
// subclasses, PhysicalSoul) still uses them.
static SqueezilPhysics *&gDefaultPhysics     = *(SqueezilPhysics **)0x4c7ad0;   // points at the block at 0x4c7828
static SqueezilPhysics *const kDefaultPhysicsBlock = (SqueezilPhysics *)0x4c7828;
static int &gPhysicsRead                     = *(int *)0x4be788;
static PhysicalSettings &gPhysicalSettings  = *(PhysicalSettings *)0x4c7768;    // PhysicalSoul's
static float &gLevelNutSprayPercent          = *(float *)0x4c7820;
static float &gSpitNutSprayPercent           = *(float *)0x4c7b0c;

static void **const kSqueezilSoulVtable      = (void **)0x49f74c;
static void **const kSqueezilSoulGrantVtable = (void **)0x49f72c;
static void **const kGrantMessageMapVtable   = (void **)0x49f844;
static void **const kMessageMapVtable        = (void **)0x49d2b4;

static const char *const kFoodNut = "FoodNutGoodie";
static const char *const kSpitNut = "SpitNutGoodie";

// ---- the state tables ----

// Whether the body counts as on the ground in each state (the 24th entry is padding)                                   @0049ed40
const int kStateOnGround[SS_COUNT + 1] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0
};

// Which state may follow which: [from][to]                                                                             @0049eda0
const int kStateMayFollow[SS_COUNT][SS_COUNT] = {
    //   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22
    { 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0 },   // 0  stand
    { 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },   // 1  lie down
    { 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0 },   // 2  get up
    { 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0 },   // 3  walk
    { 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0 },   // 4  run
    { 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0 },   // 5  skid
    { 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0 },   // 6  slip
    { 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0 },   // 7  attack
    { 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },   // 8  stumble
    { 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0 },   // 9  scamper
    { 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0 },   // 10 scamper stop
    { 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0 },   // 11 tumble
    { 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 },   // 12 jump
    { 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0 },   // 13 hop start
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0 },   // 14 hop
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0 },   // 15 skihop
    { 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0 },   // 16 spit
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 },   // 17 fly
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 },   // 18 dive
    { 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0 },   // 19 land
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1 },   // 20 dying in the air
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },   // 21 dying
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },   // 22 dead
};

// The body's clip in each state (SqueezilMode)                                                                         @0049f5e8
const int kStateClip[SS_COUNT] = {
    MODE_STOP, MODE_SLEEP_1, MODE_SLEEP_3, MODE_WALK, MODE_RUN, MODE_SKID_1, MODE_SKID, MODE_SLIDE_1, MODE_STUMBLE_1,
    MODE_SCAMPER, MODE_SCAMPER_STOP, MODE_TUMBLE, MODE_JUMP_1, MODE_HOP_1, MODE_HOP_4, MODE_HOP_4, MODE_SPIT_1, MODE_FLY,
    MODE_CANNON, MODE_SCAMPER, MODE_DEAD_SHUDDER, MODE_STOP, MODE_NONE
};

// @00446dd0
float PhysicalSettings::SetGravity(float g)
{
    gravity = Vector3f(0.0f, g, 0.0f);
    down = Vector3f(0.0f, g < 0.0f ? -1.0f : 1.0f, 0.0f);
    gravityStrength = fabsf(g);
    return g;
}

// The ini value made positive (the file gives decelerations as negative numbers, and the angles either way)
static float Size(const IniReader &ini, const char *key, float def)
{
    return fabsf(ini.GetFloat(key, def));
}

// @0044f700 The keys are read in the original's order: several defaults are values read just before.
void ReadSqueezilPhysics()
{
    IniReader ini;
    ini.SetSection("Physics");
    SqueezilPhysics *p = gDefaultPhysics;
    p->physical = gPhysicalSettings;

    p->walk.acceleration = Size(ini, "WalkAcceleration", 1.5f);
    p->walk.deceleration = Size(ini, "WalkDeceleration", -3.0f);
    p->walk.speed        = Size(ini, "WalkSpeed", 2.0f);
    p->walk.turnAccel    = Size(ini, "WalkTurnAccel", 120.0f);
    p->walk.turnDecel    = Size(ini, "WalkTurnDecel", -140.0f);
    p->walk.turnRate     = Size(ini, "WalkTurnRate", 150.0f);

    p->run.acceleration  = Size(ini, "RunAcceleration", 2.0f);
    p->run.deceleration  = Size(ini, "RunDeceleration", -3.0f);
    p->run.speed         = Size(ini, "RunSpeed", 3.0f);
    p->run.turnAccel     = Size(ini, "RunTurnAccel", p->walk.turnAccel);
    p->run.turnDecel     = Size(ini, "RunTurnDecel", p->walk.turnDecel);
    p->run.turnRate      = Size(ini, "RunTurnRate", p->walk.turnRate);

    p->skid = p->walk;
    p->skid.deceleration = Size(ini, "SkidDeceleration", 10.0f);

    p->scamper.acceleration = Size(ini, "ScamperAcceleration", 2.0f);
    p->scamper.deceleration = Size(ini, "ScamperDeceleration", -3.0f);
    p->scamper.speed        = Size(ini, "ScamperSpeed", 3.0f);
    p->scamper.turnAccel    = Size(ini, "ScamperTurnAccel", p->walk.turnAccel);
    p->scamper.turnDecel    = Size(ini, "ScamperTurnDecel", p->walk.turnDecel);
    p->scamper.turnRate     = Size(ini, "ScamperTurnRate", p->walk.turnRate);

    FlightModel &fly = p->fly;
    fly.diveAngle           = Size(ini, "DiveAngle", 42.0f);
    fly.climbAngle          = Size(ini, "ClimbAngle", -22.0f);
    fly.diveAdjustRate      = Size(ini, "DiveAdjustRate", 30.0f);
    fly.turnAccel           = Size(ini, "FlyTurnAccel", 10.0f);
    fly.turnDecel           = Size(ini, "FlyTurnDecel", -20.0f);
    fly.turnRate            = Size(ini, "FlyTurnRate", 30.0f);
    fly.bank                = Size(ini, "FlyBank", 60.0f);
    fly.pitchCorrectionRate = Size(ini, "PitchCorrectionRate", 45.0f);
    fly.controlCutoff       = ini.GetFloat("FlyControlCutoff", 10.0f);
    fly.speedFactor         = ini.GetFloat("FlySpeedFactor", 1.0f);
    fly.dragZExp            = ini.GetFloat("DragZExp", 1.0f);
    fly.dragZMult           = ini.GetFloat("DragZMult", 0.01f);
    fly.dragYExp            = ini.GetFloat("DragYExp", 1.0f);
    fly.dragYMult           = ini.GetFloat("DragYMult", 0.1f);
    fly.dragXExp            = ini.GetFloat("DragXExp", 1.0f);
    fly.dragXMult           = ini.GetFloat("DragXMult", 1.0f);
    fly.liftExp             = ini.GetFloat("LiftExp", 1.0f);
    fly.liftMult            = ini.GetFloat("LiftMult", 0.45f);
    fly.aoaAdjustMult       = ini.GetFloat("AOAAdjustMult", 5.0f);
    fly.stallAdjustMult     = ini.GetFloat("StallAdjustMult", 8.0f);
    fly.tuckLiftFactor      = ini.GetFloat("TuckLiftFactor", 0.2f);
    fly.climbLiftFactor     = ini.GetFloat("ClimbLiftFactor", 1.6f);
    fly.climbPitchFactor    = ini.GetFloat("ClimbPitchFactor", 1.35f);

    FlightModel &dive = p->dive;
    dive = fly;
    dive.dragZExp      = ini.GetFloat("DiveDragZExp", 1.0f);
    dive.dragZMult     = ini.GetFloat("DiveDragZMult", 0.6f);
    dive.dragYExp      = ini.GetFloat("DiveDragYExp", 1.0f);
    dive.dragYMult     = ini.GetFloat("DiveDragYMult", 0.6f);
    dive.dragXExp      = ini.GetFloat("DiveDragXExp", 1.0f);
    dive.dragXMult     = ini.GetFloat("DiveDragXMult", 1.0f);
    dive.liftExp       = ini.GetFloat("DiveLiftExp", 1.0f);
    dive.liftMult      = ini.GetFloat("DiveLiftMult", 0.0f);
    dive.aoaAdjustMult = ini.GetFloat("DiveAOAAdjustMult", 8.0f);
    dive.stallAdjustMult = 0.0f;

    p->tumble = dive;
    p->tumbleDuration = ini.GetFloat("TumbleDuration", 1.5f);
    p->tumble.aoaAdjustMult = 0.0f;

    JumpModel &jump = p->jump;
    jump.upSpeed        = ini.GetFloat("JumpUpSpeed", 3.0f);
    jump.fwdSpeed       = ini.GetFloat("JumpFwdSpeed", 6.0f);
    jump.upDistance     = ini.GetFloat("JumpUpDistance", 0.1f);
    jump.maxLinearSpeed = ini.GetFloat("JumpMaxLinearSpeed", 5.0f);
    jump.linearAccel    = ini.GetFloat("JumpLinearAccel", 2.0f);
    jump.dragXMult      = ini.GetFloat("JumpDragXMult", dive.dragXMult);
    jump.dragYMult      = ini.GetFloat("JumpDragYMult", dive.dragYMult);
    jump.dragZMult      = ini.GetFloat("JumpDragZMult", dive.dragZMult);
    jump.aoaAdjustMult  = ini.GetFloat("JumpAOAAdjustMult", 0.0f);
    jump.gravityFactor  = ini.GetFloat("JumpGravityFactor", 1.0f);

    JumpModel &hop = p->hop;
    hop.upSpeed        = ini.GetFloat("HopUpSpeed", 6.0f);
    hop.fwdSpeed       = ini.GetFloat("HopFwdSpeed", 4.0f);
    hop.upDistance     = ini.GetFloat("HopUpDistance", 0.1f);
    hop.maxLinearSpeed = ini.GetFloat("HopMaxLinearSpeed", jump.maxLinearSpeed);
    hop.linearAccel    = ini.GetFloat("HopLinearAccel", 2.0f);
    hop.dragXMult      = ini.GetFloat("HopDragXMult", dive.dragXMult);
    hop.dragYMult      = ini.GetFloat("HopDragYMult", dive.dragYMult);
    hop.dragZMult      = ini.GetFloat("HopDragZMult", dive.dragZMult);
    hop.aoaAdjustMult  = ini.GetFloat("HopAOAAdjustMult", 0.0f);
    hop.gravityFactor  = ini.GetFloat("HopGravityFactor", 1.0f);

    JumpModel &skihop = p->skihop;
    skihop.upSpeed        = ini.GetFloat("SkihopUpSpeed", jump.upSpeed);
    skihop.fwdSpeed       = ini.GetFloat("SkihopFwdSpeed", jump.fwdSpeed);
    skihop.upDistance     = ini.GetFloat("SkihopUpDistance", jump.upDistance);
    skihop.maxLinearSpeed = ini.GetFloat("SkihopMaxLinearSpeed", jump.maxLinearSpeed);
    skihop.linearAccel    = ini.GetFloat("SkihopLinearAccel", jump.linearAccel);
    skihop.dragXMult      = ini.GetFloat("SkihopDragXMult", dive.dragXMult);
    skihop.dragYMult      = ini.GetFloat("SkihopDragYMult", dive.dragYMult);
    skihop.dragZMult      = ini.GetFloat("SkihopDragZMult", dive.dragZMult);
    skihop.aoaAdjustMult  = ini.GetFloat("SkihopAOAAdjustMult", 0.0f);
    skihop.gravityFactor  = ini.GetFloat("SkihopGravityFactor", 1.0f);

    LeverMotion &slip = p->slip;
    slip.speedCutoff       = Size(ini, "SlipSpeedCutoff", p->walk.speed * 0.25f);
    slip.distanceCutoff    = Size(ini, "SlipDistanceCutoff", 0.5f);
    slip.turnAccel         = Size(ini, "SlipTurnAccel", 0.0f);
    slip.turnDecel         = Size(ini, "SlipTurnDecel", 0.0f);
    slip.turnRate          = Size(ini, "SlipTurnRate", 0.0f);
    slip.minSpeed          = 1.0f;
    slip.pushIncrement     = 0.0f;
    slip.frictionScale     = 1.0f;
    slip.controlSpeed      = ini.GetFloat("SlipControlSpeed", 0.0f);
    slip.leverArm          = ini.GetFloat("SlipLeverArm", 1.0f);
    slip.leverWeightFactor = ini.GetFloat("SlipLeverWeightFactor", 0.5f);

    LeverMotion &attack = p->attack;
    attack = slip;
    attack.turnAccel         = Size(ini, "AttackTurnAccel", 90.0f);
    attack.turnDecel         = Size(ini, "AttackTurnDecel", 75.0f);
    attack.turnRate          = Size(ini, "AttackTurnRate", 100.0f);
    attack.minSpeed          = Size(ini, "AttackMinSpeed", p->walk.speed * 0.25f);
    attack.pushIncrement     = Size(ini, "AttackPushIncrement", 1.0f);
    attack.frictionScale     = ini.GetFloat("AttackFrictionScale", 0.25f);
    attack.controlSpeed      = Size(ini, "AttackControlSpeed", p->run.speed);
    attack.leverArm          = ini.GetFloat("AttackLeverArm", 1.0f);
    attack.leverWeightFactor = ini.GetFloat("AttackLeverWeightFactor", 0.5f);

    p->clawGrip           = ini.GetFloat("ClawGrip", 0.3f);
    p->crouchGrip         = ini.GetFloat("CrouchGrip", 0.3f);
    p->spitVelocity       = ini.GetFloat("SpitVelocity", 10.0f);
    p->walkToScamperAngle = ini.GetFloat("WalkToScamperAngle", 70.0f);
    p->scamperToWalkAngle = ini.GetFloat("ScamperToWalkAngle", 60.0f);
    p->cornerAngle        = ini.GetFloat("CornerAngle", 70.0f);
    p->unknown288         = 1;
    p->maxLandingSpeed    = ini.GetFloat("MaxLandingSpeed", 100.0f);
    p->fwdTapFactor       = ini.GetFloat("FwdTapFactor", 1.0f);
    p->rotTapFactor       = ini.GetFloat("RotTapFactor", 1.0f);
    gLevelNutSprayPercent = ini.GetFloat("LevelNutSprayPercent", 1.0f);
    gSpitNutSprayPercent  = ini.GetFloat("SpitNutSprayPercent", 1.0f);
    p->minStumbleSpeed    = ini.GetFloat("MinStumbleSpeed", 25.0f);
    p->maxWalkSpeed       = ini.GetFloat("MaxWalkSpeed", 10.0f);
    p->maxRunSpeed        = ini.GetFloat("MaxRunSpeed", 20.0f);
}

// @00450b90
void SetFloorLevelScript(const char *name)
{
    *kFloorLevelScript = String(name);
}

// @00450c00
void SetFloorLevel(float y)
{
    *kFloorLevel = y;
}

// @00450c10
void SetMaxLandingSpeed(float speed)
{
    gDefaultPhysics->maxLandingSpeed = speed;
}

// ---- the direction history ----

// @00456da0
void DirectionHistory::Init(int n)
{
    size = n + 1;
    slots = NewArray<Vector3f>(size);
    Clear();
}

// @00456e60
void DirectionHistory::Destroy()
{
    if (size > 0 && slots) {
        DeleteArray(slots);
        slots = NULL;
        size = 0;
    }
}

// @00456ef0
void DirectionHistory::Clear()
{
    sum = Vector3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < size; i++)
        slots[i] = sum;
    next = 0;
    added = 0;
}

// @00456f40 The new direction takes the next slot; the one after it (the oldest) drops out of the sum.
Vector3f *DirectionHistory::Add(const Vector3f *direction)
{
    slots[next] = *direction;
    int after = next + 1;
    if (size == after)
        after = 0;
    sum = sum + *direction - slots[after];
    next = after;
    added++;
    return &sum;
}

// ---- construction ----

// @0044f1a0
SqueezilSoul *SqueezilSoul::Init(SqueezilBody *theBody, Possessions *thePossessions, void *thePlace)
{
    PhysicalSoulInit(this);
    finished = 0;                       // GrantMessageMap's constructor (inlined)
    spitPhase = 0;
    grantVtbl = kMessageMapVtable;
    grantVtbl = kGrantMessageMapVtable;
    directions.Init(4);
    state = SS_STAND;
    frozen = 0;
    canSlip = 1;
    pitchTarget = kNoInput;
    limitLanding = -1;
    physics = (SqueezilPhysics *)Operator_New(sizeof(SqueezilPhysics));
    unknown4c = 0;
    spitRequested = 0;
    leverSpin = 0.0f;
    new (&surface.typeName) String();
    new (&surface.value) String();
    possessions = thePossessions;
    place = thePlace;
    body = theBody;
    vtbl = kSqueezilSoulVtable;
    grantVtbl = kSqueezilSoulGrantVtable;
    if (!gPhysicsRead) {
        gPhysicsRead = 1;
        gDefaultPhysics = kDefaultPhysicsBlock;
        ReadSqueezilPhysics();
    }
    *physics = *gDefaultPhysics;        // (not checked for NULL)
    surface.type = 0;
    surface.typeName = String(" none");
    surface.value = String("none");
    BodyCall::SetOnGround(theBody, 1);
    return this;
}

// @0044f600 The base classes' destructors only put back their own vtables.
void SqueezilSoul::Destruct()
{
    vtbl = kSqueezilSoulVtable;
    grantVtbl = kSqueezilSoulGrantVtable;
    if (possessions) {
        CallVirtual<void>(possessions, 4, 1);
        possessions = NULL;
    }
    if (physics) {
        Operator_Delete(physics);       // (its destructor 0x4052b0 does nothing)
        physics = NULL;
    }
    surface.value.~String();            // @0044f590
    surface.typeName.~String();
    directions.Destroy();
    grantVtbl = kMessageMapVtable;      // GrantMessageMap @00450980
    vtbl = kMessageMapVtable;           // PhysicalSoul @00450e40, down to MessageMap
}

// @0044f570
SqueezilSoul *SqueezilSoul::DestroyAndFree(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// ---- small virtual functions ----

// @004503f0
void *SqueezilSoul::GetInterface(int which)
{
    if (which == 1)
        return &grantVtbl;             // (the original also checks this for NULL)
    return NULL;
}

// @00450410
int SqueezilSoul::IsActive()
{
    return state != SS_DEAD && frozen == 0;
}

// @004509d0
int SqueezilSoul::IsAttacking()
{
    if (state == SS_ATTACK)
        return 1;
    if (state == SS_FLY && BodyCall::GetMode(SoulCall::Body(this)) == MODE_CANNON)
        return 1;
    return 0;
}

// @004569a0
void SqueezilSoul::Unfreeze()
{
    frozen--;
    if (frozen < 0)
        frozen = 0;
}

// @00450310
Possessions *SqueezilSoul::TakePossessions()
{
    Possessions *p = possessions;
    possessions = NULL;
    return p;
}

// @00404fe0
SqueezilPhysics *SqueezilSoul::GetDefaultPhysics()
{
    return gDefaultPhysics;
}

// @00404ff0 The table is read as one flat block, as the original does (it is never asked for SS_NONE here).
int SqueezilSoul::CanEnter(int newState)
{
    return (&kStateMayFollow[0][0])[state * SS_COUNT + newState];
}

// @00405020
int SqueezilSoul::IsDying()
{
    return kStateMayFollow[state][SS_DEAD];
}

// @00456590
int SqueezilSoul::GroundStateForSpeed(float speed)
{
    if (physics->minStumbleSpeed < speed)
        return SS_STUMBLE;
    if (physics->maxWalkSpeed < speed)
        return SS_RUN;
    if (speed > 0.01f)
        return SS_WALK;
    return SS_STAND;
}

// @00456b20
void SqueezilSoul::SetSpitStrength(float strength)
{
    if (spitRequested)
        spitRequested = 0;
    else
        spitStrength = strength;
}

// @00456b90 Copies the settings of the state's way of moving behind the input.
void SqueezilSoul::GetGroundMotion(int forState, GroundControl *c)
{
    switch (forState) {
    case SS_STAND: case SS_LIE_DOWN: case SS_GET_UP: case SS_WALK: case SS_HOP_START: case SS_SPIT: case SS_LAND:
    case SS_DYING: case SS_DEAD:
        c->ground = physics->walk;
        break;
    case SS_RUN:
        c->ground = physics->run;
        break;
    case SS_SKID:
        c->ground = physics->skid;
        break;
    case SS_SLIP: case SS_STUMBLE:
        c->lever = physics->slip;
        break;
    case SS_ATTACK:
        c->lever = physics->attack;
        break;
    case SS_SCAMPER: case SS_SCAMPER_STOP:
        c->ground = physics->scamper;
        break;
    }
}

// @00456ca0
void SqueezilSoul::GetAirMotion(int forState, AirControl *c)
{
    switch (forState) {
    case SS_TUMBLE:
        c->flight = physics->tumble;
        break;
    case SS_JUMP:
        c->jump = physics->jump;
        break;
    case SS_HOP:
        c->jump = physics->hop;
        break;
    case SS_SKIHOP:
        c->jump = physics->skihop;
        break;
    case SS_FLY:
        c->flight = physics->fly;
        break;
    case SS_DIVE:
        c->flight = physics->dive;
        break;
    }
}

// ---- the state machine ----

// @00450320 Switching state also picks the body's clip and whether it counts as on the ground. A body leaving the ground takes
// its flight angles from its matrix.
void SqueezilSoul::SetState(int newState, int blend)
{
    if (newState == state || newState == SS_NONE)
        return;
    previousState = state;
    state = newState;
    if (newState != SS_DEAD)
        BodyCall::SetMode(SoulCall::Body(this), kStateClip[newState], blend);
    int onGround = kStateOnGround[newState];
    int wasOnGround = kStateOnGround[previousState];
    BodyCall::SetOnGround(SoulCall::Body(this), onGround);
    if (wasOnGround && !onGround)
        BodyCall::UpdateOrientation(SoulCall::Body(this));
    if (state == SS_LAND) {
        limitLanding = 0;
        if (previousState != SS_TUMBLE)
            limitLanding = 1;
    }
    if (state != SS_TUMBLE)
        tumbleTime = 0.0f;
    if (state != SS_HOP_START && state != SS_HOP)
        hopStrength = -1.0f;
    if (state != SS_SPIT)
        spitStrength = -1.0f;
}

// @00452bb0
void SqueezilSoul::StartTumble()
{
    if (SoulCall::CanEnter(this, SS_TUMBLE)) {
        SoulCall::SetState(this, SS_TUMBLE, 0);
        tumbleTime = physics->tumbleDuration;
        SoulCall::ResetSurface(this);
    }
}

// @004506d0 Knocked over (a tumble in the air, a stumble on the ground), and maybe made to drop nuts.
void SqueezilSoul::Hit(int knock, int sprayNuts, int unused)
{
    if (knock) {
        if (!BodyCall::IsOnGround(SoulCall::Body(this))) {
            if (state != SS_DYING_IN_AIR) {
                SoulCall::SetState(this, SS_TUMBLE, 0);
                tumbleTime = physics->tumbleDuration;
            }
        } else if (state != SS_DYING) {
            SoulCall::SetState(this, SS_STUMBLE, 0);
        }
    }
    if (sprayNuts)
        SoulCall::SprayNuts(this);
    spitPhase = 0;
}

// @00450a00 how: 0 at once (no clip), 1 faint, 2 drown, 3 shudder, 9 keep the current clip; 4-8 only start dying.
void SqueezilSoul::Die(int how)
{
    int current = SoulCall::GetState(this);
    if (current == SS_DEAD)
        return;
    if (how != 0 && (current == SS_DYING || current == SS_DYING_IN_AIR))
        return;
    if (*kLoopingSoundPlaying)
        StopLoopingSounds();
    InventoryRemoveNuts(SoulCall::GetInventory(this));
    stateAtDeath = SoulCall::GetState(this);
    int dying = BodyCall::IsOnGround(SoulCall::Body(this)) ? SS_DYING : SS_DYING_IN_AIR;
    switch (how) {
    case 0:
        finished = 1;
        if (SoulCall::GetState(this) != dying && SoulCall::GetState(this) != SS_DEAD)
            SoulCall::SetState(this, dying, 0);
        SoulCall::SetState(this, SS_DEAD, 0);
        BodyCall::SetMode(SoulCall::Body(this), MODE_NONE);
        break;
    case 1:
        SoulCall::SetState(this, dying, 0);
        BodyCall::SetMode(SoulCall::Body(this), MODE_DEAD_FAINT_1);
        break;
    case 2:
        SoulCall::SetState(this, dying, 0);
        BodyCall::SetMode(SoulCall::Body(this), MODE_DEAD_DROWN);
        break;
    case 3:
        SoulCall::SetState(this, dying, 0);
        BodyCall::SetMode(SoulCall::Body(this), MODE_DEAD_SHUDDER);
        break;
    case 9: {
        int mode = BodyCall::GetMode(SoulCall::Body(this));
        SoulCall::SetState(this, dying, 0);
        BodyCall::SetMode(SoulCall::Body(this), mode);
        break;
    }
    }
}

// ---- other creatures and goodies ----

// @00450430 Another creature touched this one. A creature that takes touches itself (a goodie: its GrantMessageMap part says so in
// slot 3 and handles it in slot 5) is left to do so; otherwise the two bump (PhysicalSoul's collision).
void SqueezilSoul::Touch(int otherId)
{
    LocationSimulation *sim = (LocationSimulation *)GetCurrentSimulation();
    void *other = sim->GetSoul(otherId);
    void *grant = DynamicCast(CallVirtual<void *>(other, SQS_GET_INTERFACE, 1), kMessageMapType, kGrantMessageMapType);
    int id = sim->FindIdBySoul(this);
    if (!grant)
        return;
    if (CallVirtual<int>(grant, 3, (int)(id != 0)) && CallVirtual<int>(grant, 5, id))
        return;
    PhysicalCollision(sim->FindIdByActor(SoulCall::Body(this)), otherId);
}

// @004504f0 (GrantMessageMap slot 4) Nuts go into their lists; any other goodie is held, if nothing is held yet. The place hears
// of every pickup.
int SqueezilSoul::Grant(const char *goodie, int value)
{
    PlayEffect("pickupEffect", BodyPosition(this));
    if (strcmp(goodie, kFoodNut) == 0 || strcmp(goodie, kSpitNut) == 0) {
        InventoryAdd(SoulCall::GetInventory(this), goodie, value);
    } else {
        Inventory *inv = SoulCall::GetInventory(this);
        if (strcmp(InventoryHeldName(inv)->c_str(), "") != 0)
            return 0;
        *InventoryHeldName(inv) = String(goodie);
        *InventoryHeldValue(inv) = value;
    }
    GoodieGranted(GetPlaceGoodies(place), goodie, this, place);
    return 1;
}

// One kind of nut sprayed: a share of the count (at least one), each dropped at the squeezil
static void SprayNutsOf(SqueezilSoul *soul, Inventory *inv, const char *goodie, float percent)
{
    int count = InventoryCount(inv, goodie);
    if (!count)
        return;
    int drop = 1;
    if (count > 1)
        drop = (int)(count * percent);
    Array<int> *items = InventoryItems(inv, goodie);
    for (int i = 0; i < drop; i++) {
        Matrix16 at = *(Matrix16 *)BodyCall::Matrix(SoulCall::Body(soul));
        DropGoodie(GetPlaceGoodies(soul->place), items->At(i), goodie, at);
    }
    for (int i = drop; i > 0; i--)
        InventoryRemove(inv, goodie, 0);
}

// @00450740
void SqueezilSoul::SprayNuts()
{
    Inventory *inv = SoulCall::GetInventory(this);
    SprayNutsOf(this, inv, kFoodNut, gLevelNutSprayPercent);
    SprayNutsOf(this, inv, kSpitNut, gSpitNutSprayPercent);
}

// @004569c0 Standing still starts a spit (its strength comes later, SetSpitStrength); walking and attacking start a full one;
// on the move a nut flies at once.
int SqueezilSoul::Spit()
{
    if (frozen) {
        spitRequested = 1;
        return SS_NONE;
    }
    switch (SoulCall::GetState(this)) {
    case SS_STAND:
        spitStrength = -1.0f;
        return SS_SPIT;
    case SS_WALK: case SS_ATTACK:
        spitStrength = 1.0f;
        spitRequested = 1;
        return SS_SPIT;
    case SS_RUN: case SS_SLIP: case SS_SCAMPER: case SS_SCAMPER_STOP: case SS_JUMP: case SS_HOP: case SS_FLY:
        PlayEffect("spitEffect", BodyPosition(this));
        SoulCall::SpitNut(this, physics->spitVelocity);
        return SS_NONE;
    default:
        spitRequested = 1;
        return SS_NONE;
    }
}

// @004530f0 Throws the first spit nut forward from the mouth (a little up and forward of the body on the ground, a step ahead
// in the air), with the squeezil's own velocity added.
void SqueezilSoul::SpitNut(float speed)
{
    Inventory *inv = SoulCall::GetInventory(this);
    if (InventoryCount(inv, kSpitNut) == 0)
        return;
    Array<int> *items = InventoryItems(SoulCall::GetInventory(this), kSpitNut);
    SqueezilBody *b = SoulCall::Body(this);
    Vector3f up = Row(BodyCall::Matrix(b), 1);
    Vector3f forward = Row(BodyCall::Matrix(SoulCall::Body(this)), 2);
    Vector3f velocity = BodyCall::Direction(SoulCall::Body(this)) * BodyCall::Speed(SoulCall::Body(this));
    velocity = velocity + Row(BodyCall::Matrix(SoulCall::Body(this)), 2) * speed;
    Vector3f from;
    if (BodyCall::IsOnGround(SoulCall::Body(this)))
        from = BodyCall::Position(SoulCall::Body(this)) + up * 0.2f + forward * 0.1f;
    else
        from = BodyCall::Position(SoulCall::Body(this)) + forward * 1.0f;
    Matrix16 at = *(Matrix16 *)BodyCall::Matrix(SoulCall::Body(this));
    SetTranslation(at.m, from);
    float throwSpeed = Normalize(velocity);
    ThrowGoodie(GetPlaceGoodies(place), items->At(0), at, ToFloat3(velocity), throwSpeed);
    CallVirtual<void>(items, 2, 0);     // RemoveAt(0)
}

// ---- the surface type ----

// @00451500
void SqueezilSoul::ResetSurface()
{
    SurfaceInfo old = surface;
    surface.type = 0;
    surface.typeName = String(" none");
    surface.value = String("none");
    NavSurfaceTypeChanged(GetPlaceSurfaces(place), this, BodyCall::GetContact(SoulCall::Body(this)), &old, &surface);
}

// @00451270 The face's type number is in the world's record of the surface copy. A new type takes its name and the face's
// property of that name; with the same type, only a change of that property counts (and is logged).
void SqueezilSoul::UpdateSurface(Contact contact)
{
    char *faces = *(char **)((char *)WorldSurface(GetWorld(), contact.face) + 0xc);
    int type = *(int *)(faces + contact.second * 0x28 + 0x24);
    void *surfaces = GetPlaceSurfaces(place);
    if (type != surface.type) {
        SurfaceInfo old = surface;
        surface.type = type;
        surface.typeName = NavSurfaceTypeName(GetPlaceSurfaces(place), &contact);
        surface.value = NavSurfaceProperties(GetPlaceSurfaces(place), &contact)->Get(surface.typeName);
        NavSurfaceTypeChanged(GetPlaceSurfaces(place), this, &contact, &old, &surface);
        return;
    }
    String name = NavSurfaceTypeName(surfaces, &contact);
    surface.typeName = name;
    String value = NavSurfaceProperties(GetPlaceSurfaces(place), &contact)->Get(name);
    if (StringCompare(&value, &surface.value) != 0) {
        GameLog("Surface type changed to %s!\n", value.c_str());
        SurfaceInfo old = surface;
        surface.value = value;
        NavSurfaceTypeChanged(GetPlaceSurfaces(place), this, &contact, &old, &surface);
    }
}

// ---- the frame ----

static GroundControl &gGroundControl = *(GroundControl *)0x4c7ad8;
static AirControl &gAirControl = *(AirControl *)0x4c7b10;

// @00450e90 A squeezil whose surface copy went away starts tumbling. Then the subclass says what it wants, and the current state's
// handler acts on it; at the end the body's matrix is squared up again.
void SqueezilSoul::Update()
{
    SqueezilBody *b = SoulCall::Body(this);
    int onGround = BodyCall::IsOnGround(b);
    if (onGround && NavSurfaceIsGone(GetPlaceSurfaces(place), BodyCall::GetContact(SoulCall::Body(this)))) {
        SoulCall::StartTumble(this);
        onGround = BodyCall::IsOnGround(SoulCall::Body(this));
    }
    int current = SoulCall::GetState(this);
    int next = SS_NONE;
    if (onGround) {
        SoulCall::ReadGroundInput(this, &gGroundControl, &next);
        SoulCall::AdjustNextState(this, &next);
        int slot = -1;
        switch (current) {
        case SS_STAND: case SS_SCAMPER_STOP: {
            SqueezilBody *body = SoulCall::Body(this);
            int mode = BodyCall::GetMode(body);
            if (mode == MODE_STOP || BodyCall::GetMode(SoulCall::Body(this)) == MODE_SCAMPER_STOP)
                slot = SQS_GROUND_MOVE;
            else if (BodyCall::ClipEnded(SoulCall::Body(this)))
                BodyCall::SetMode(SoulCall::Body(this), MODE_STOP);
            break;
        }
        case SS_LIE_DOWN:     slot = SQS_LIE_DOWN; break;
        case SS_GET_UP:       slot = SQS_GET_UP; break;
        case SS_WALK: case SS_RUN: case SS_SCAMPER:
                              slot = SQS_GROUND_MOVE; break;
        case SS_SKID:         slot = SQS_SKID; break;
        case SS_SLIP:         slot = SQS_SLIP; break;
        case SS_ATTACK:       slot = SQS_ATTACK; break;
        case SS_STUMBLE:      slot = SQS_STUMBLE; break;
        case SS_HOP_START:    slot = SQS_HOP_START; break;
        case SS_SPIT:         slot = SQS_SPIT_STANDING; break;
        case SS_LAND:         slot = SQS_LAND; break;
        case SS_DYING:        slot = SQS_DYING; break;
        case SS_DEAD:         SoulCall::SetState(this, next, 0); break;
        }
        if (slot >= 0)
            SoulCall::Handle(this, slot, &gGroundControl, next);
    } else {
        SoulCall::ReadAirInput(this, &gAirControl, &next);
        int slot = -1;
        switch (current) {
        case SS_TUMBLE:          slot = SQS_TUMBLE; break;
        case SS_JUMP:            slot = SQS_JUMP_AIR; break;
        case SS_HOP:             slot = SQS_HOP_AIR; break;
        case SS_SKIHOP:          slot = SQS_SKIHOP_AIR; break;
        case SS_FLY: case SS_DIVE:
                                 slot = SQS_FLY; break;
        case SS_DYING_IN_AIR:    slot = SQS_DYING_IN_AIR; break;
        case SS_DEAD:            SoulCall::SetState(this, next, 0); break;
        }
        if (slot >= 0)
            SoulCall::Handle(this, slot, &gAirControl, next);
    }
    float m[16];
    memcpy(m, BodyCall::Matrix(SoulCall::Body(this)), sizeof m);
    OrthonormalizeMatrix(m);
    BodyCall::SetMatrix(SoulCall::Body(this), m);
}
