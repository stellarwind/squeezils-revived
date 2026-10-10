// SqueezilRoboSoul: see SqueezilRoboSoul.h. Reconstructed from SQZ.EXE 0x4043a0-0x4053d0.
#include "SqueezilRoboSoul.h"
#include "SqueezilSoulInternal.h"
#include "Behavior.h"
#include "CreatureTemplate.h"
#include "Simulation.h"
#include "Original.h"
#include "VirtualCall.h"
#include <new>

static_assert(offsetof(SqueezilRoboSoul, records) == 0xa4, "SqueezilRoboSoul layout");
static_assert(offsetof(SqueezilRoboSoul, behaviour) == 0xec, "SqueezilRoboSoul layout");
static_assert(offsetof(SqueezilRoboSoul, pendingFunction) == 0xf8, "SqueezilRoboSoul layout");

static void **const kRoboSoulVtable  = (void **)0x49d18c;
static void **const kRoboGrantVtable = (void **)0x49d16c;
static const int kSlotSetBehaviour = 60;        // the one virtual function this class adds (SqueezilSoul has slots 0-59)
static const int kSlotKind = 8;                 // Soul slot 8: what kind of creature it is (RoboKind; 4 and 6 do not count)

// The creatures' sense radii are the CreatureTemplate's +0x14 (player), +0x18 (robo), +0x1c (goodie)
static float SenseRadius(CreatureTemplate *c, int kind) { return c->senseRadius[kind]; }

// The creature soul behind an id, as a Soul that has a kind
static void *KindSoul(int id)
{
    return DynamicCast(((LocationSimulation *)GetCurrentSimulation())->GetSoul(id), kSoulType, kSimulationSoulType);
}

// @00448370
void AwarenessRecord::Offer(const AwarenessRecord *other)
{
    if (!stale && distance <= other->distance)
        return;
    id = other->id;
    distance = other->distance;
    position = other->position;
    stale = 0;
}

// @004043a0
SqueezilRoboSoul *SqueezilRoboSoul::Init(SqueezilBody *bodyArg, Possessions *possessionsArg, void *placeArg, CreatureTemplate *characterArg)
{
    SqueezilSoul::Init(bodyArg, possessionsArg, placeArg);
    step = 0;
    retarget = 0;
    senseKind = kNoSense;
    lastSenseKind = kNoSense;
    wasOnGround = 1;
    for (int i = 0; i < 3; i++) {               // the records' constructor @00404f10
        records[i].id = -1;
        records[i].stale = 1;
    }
    behaviour = NULL;
    character = characterArg;
    vtbl = kRoboSoulVtable;
    mode = 7;
    grantVtbl = kRoboGrantVtable;
    ClearSeen();
    pendingFunction = NULL;         // (the original leaves this field as the memory had it; 0 is what it needs)
    return this;
}

// @00404510
void SqueezilRoboSoul::Destruct()
{
    vtbl = kRoboSoulVtable;
    grantVtbl = kRoboGrantVtable;
    if (behaviour)
        CallVirtual<void>(behaviour, 0, 1);
    SqueezilSoul::Destruct();
}

// @004044a0 slot 0
SqueezilRoboSoul *SqueezilRoboSoul::DestroyAndFree(int flags)
{
    Destruct();
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @004053d0 slot 8
int SqueezilRoboSoul::GetKind() const { return finished ? 6 : 1; }

// @004046f0 slot 7
void SqueezilRoboSoul::ResetSenses()
{
    for (int i = 0; i < 3; i++)
        records[i].Reset();
}

// @00404720 slot 6 A creature is near: kept if it is the nearest of its kind
void SqueezilRoboSoul::Sense(const AwarenessRecord *seen)
{
    int kind = CallVirtual<int>(KindSoul(seen->id), kSlotKind);
    if (kind < 3)
        records[kind].Offer(seen);
}

// @00404770
int SqueezilRoboSoul::IsNear(int kind)
{
    if (records[kind].id == -1)
        return 0;
    float radius = SenseRadius(character, kind);
    return radius * radius > records[kind].distance;
}

// @004047c0
int SqueezilRoboSoul::TargetGone(int kind)
{
    if (kind >= 3)
        return 0;
    int id = records[kind].id;
    if (id == -1)
        return 0;
    if (!((LocationSimulation *)GetCurrentSimulation())->GetActor(id))
        return 1;
    return CallVirtual<int>(KindSoul(id), kSlotKind) == 6;
}

// @00404900
void SqueezilRoboSoul::ClearSeen()
{
    lastSeen[0] = lastSeen[1] = lastSeen[2] = -1;
}

// @00404840 The soul's own state: dying (3, 4) or leaving (1) and touching (2) the ground
int SqueezilRoboSoul::PickState()
{
    int theState = SoulCall::GetState(this);
    int onGround = BodyCall::IsOnGround(SoulCall::Body(this));
    if (!behaviour) {
        wasOnGround = onGround;
        return 0;
    }
    if (theState == SS_DYING || (theState == SS_DEAD && onGround))
        return 3;
    if (theState == SS_DYING_IN_AIR || (theState == SS_DEAD && !onGround))
        return 4;
    if (wasOnGround) {
        if (!onGround) {
            wasOnGround = 0;
            return 1;
        }
    } else if (onGround) {
        wasOnGround = 1;
        return 2;
    }
    return kNoActivity;
}

// @00404920 What to do about the nearest creature of a kind: a new one noticed (5..0xa by kind and ground or air), the one noticed
// lost (0xb, 0xc), or nothing (0x13). senseKind tells the Update which kind it was.
int SqueezilRoboSoul::SenseState(int kind)
{
    AwarenessRecord *record = &records[kind];
    int *last = &lastSeen[kind];
    int onGround = BodyCall::IsOnGround(SoulCall::Body(this));

    int action;                 // 0 a new creature, 1 lost it, 2 none
    if (IsNear(kind)) {
        int gone = TargetGone(kind);
        if (*last == record->id && !retarget) {
            if (gone) {
                action = 1;
                record->id = -1;
            } else {
                action = 2;
            }
        } else if (gone) {
            record->id = -1;
            action = 1 + (*last == -1);
            *last = record->id;
        } else {
            action = 0;
            *last = record->id;
        }
    } else {
        action = 1 + (*last == -1);
    }

    switch (action) {
    case 0:
        senseKind = kind;
        switch (kind) {
        case ROBO_PLAYER: return onGround ? 5 : 6;
        case ROBO_ROBO:   return onGround ? 7 : 8;
        case ROBO_GOODIE: return onGround ? 9 : 10;
        }
        return onGround;
    case 1:
        ClearSeen();
        senseKind = kNoSense;
        return onGround ? 0xb : 0xc;
    case 2:
        senseKind = kNoSense;
        return kNoActivity;
    }
    return kNoActivity;
}

// @00404a80
int SqueezilRoboSoul::StepAI()
{
    int result = kNoActivity;       // (the original reads whatever the stack held; every step sets it)
    do {
        step++;
        switch (step - 1) {
        case 0: result = PickState(); break;
        case 1: result = SenseState(0); break;
        case 2: result = SenseState(1); break;
        case 3: result = SenseState(2); break;
        case 4: step = 0; result = kNoActivity; break;
        }
    } while (result == kNoActivity && step != 0);
    return result;
}

// @00404df0 slot 5 One frame: the state machine until a state is taken, then its function runs; a soul that has nothing in mind
// (its last noticed creature is gone) settles into the ground or air stand and the default behaviour.
void SqueezilRoboSoul::UpdateRobo()
{
    for (;;) {
        int wanted = StepAI();
        if (wanted == kNoActivity)
            break;
        if (character->TrySetState(wanted, this))
            break;
    }
    step = 0;
    retarget = 0;
    character->RunPending(this);

    if (mode == 3 || mode == 4 || mode == 5) {
        int last = lastSenseKind;
        if (last == kNoSense || TargetGone(last) || records[last].id == -1) {
            int onGround = BodyCall::IsOnGround(SoulCall::Body(this));
            if (!character->TrySetState(onGround ? 0xb : 0xc, this))
                character->TrySetState(0, this);
            character->RunPending(this);
            if (mode == 3 || mode == 4 || mode == 5) {
                float args[3] = { 90.0f, 5.0f, 1.0f };
                CallVirtual<void>(this, kSlotSetBehaviour, 0, args);
            }
        }
    }
    SqueezilSoul::Update();
}

// @00404630 slot 9 Hit by a creature: the state for being hit by its kind, on the ground or in the air, runs the CreatureTemplate's function
void SqueezilRoboSoul::Hit(int knock, int sprayNuts, int byId)
{
    senseKind = CallVirtual<int>(KindSoul(byId), kSlotKind);
    int onGround = BodyCall::IsOnGround(SoulCall::Body(this));
    int wanted = kNoActivity;
    switch (senseKind) {
    case ROBO_PLAYER: wanted = onGround ? 0xd : 0xe; break;
    case ROBO_ROBO:   wanted = onGround ? 0xf : 0x10; break;
    case ROBO_GOODIE: wanted = onGround ? 0x11 : 0x12; break;
    }
    if (wanted != kNoActivity && character->TrySetState(wanted, this))
        character->RunPending(this);
    SqueezilSoul::Hit(knock, sprayNuts, byId);
}

// @004045b0 slot 39
void SqueezilRoboSoul::ReadGroundInput(GroundControl *c, int *next)
{
    CallVirtual<void>(this, SQS_GET_GROUND_MOTION, SoulCall::GetState(this), c);
    *next = CallVirtual<int>(behaviour, 1, c, (AirControl *)NULL);
}

// @004045f0 slot 40
void SqueezilRoboSoul::ReadAirInput(AirControl *c, int *next)
{
    SoulCall::GetAirMotion(this, SoulCall::GetState(this), c);
    *next = CallVirtual<int>(behaviour, 1, (GroundControl *)NULL, c);
}

// @00404b10 slot 60 Change the behaviour. The seven classes (Behavior.h; sizes in the comments) take the
// body and the soul and some of the three numbers.
void SqueezilRoboSoul::SetBehaviour(int newMode, const float *args)
{
    if (newMode == mode)
        return;
    SqueezilBody *theBody = SoulCall::Body(this);
    void *fresh = NULL;
    switch (newMode) {
    case 0: fresh = new PaceBehavior(theBody, this, args[0], args[1], args[2]); break;     // 0x30 bytes
    case 1: fresh = new WanderBehavior(theBody, this); break;                            // 0x24
    case 2: fresh = new SimpleFlyBehavior(theBody, this); break;                         // 0x18
    case 3: fresh = new ChaseBehavior(theBody, this); break;                             // 0x20
    case 4: fresh = new SpitBehavior(theBody, this, args[0], args[1]); break;            // 0x28
    case 5: fresh = new FleeBehavior(theBody, this); break;                              // 0x20
    case 6: fresh = new DeathBehavior(theBody, this, args[0]); break;                    // 0x24
    default: fresh = NULL; break;       // (the original stores an unset local here)
    }
    if (behaviour) {
        if (mode == 4 && SoulCall::GetState(this) == SS_SPIT)
            CallVirtual<void>(this, SQS_SET_SPIT_STRENGTH, 1.0f);
        if (behaviour)
            CallVirtual<void>(behaviour, 0, 1);
    }
    behaviour = fresh;
    mode = newMode;
}
