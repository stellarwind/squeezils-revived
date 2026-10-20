// Behavior: see Behavior.h. Reconstructed from SQZ.EXE 0x4022f0-0x404380.
#include "Behavior.h"
#include "SqueezilSoulInternal.h"
#include "NavMesh.h"
#include "Original.h"
#include <math.h>

static void **const kBehaviorVtable  = (void **)0x49d0bc;
static void **const kWanderVtable    = (void **)0x49d0cc;
static void **const kPaceVtable      = (void **)0x49d0ec;
static void **const kDeathVtable     = (void **)0x49d0fc;
static void **const kSimpleFlyVtable = (void **)0x49d10c;
static void **const kChaseVtable     = (void **)0x49d13c;
static void **const kSpitVtable      = (void **)0x49d14c;
static void **const kFleeVtable      = (void **)0x49d15c;

static const char kSpitNutGoodie[] = "SpitNutGoodie";    // 0x4b9334: the goodie a Spit behaviour needs one of
static const float kChaseLookAhead = 2.0f;               // 0x49d038: seconds ahead the target is aimed at, at the most
static const float kChaseRunAngle  = 50.0f;              // 0x49d04c: degrees off the target at which a chase falls back to a walk

static int Milliseconds(float seconds) { return (int)(seconds * 1000.0f); }
static bool IsZero(float f) { return f == 0.0f; }        // (the original tests the bits without the sign)

// @004022f0
Behavior::Behavior(SqueezilBody *bodyArg, SqueezilSoul *soulArg)
{
    vtbl = kBehaviorVtable;
    body = bodyArg;
    soul = soulArg;
    current = NULL;
}

// The record of the creature the soul noticed last
AwarenessRecord *Behavior::Noticed() const
{
    SqueezilRoboSoul *robo = (SqueezilRoboSoul *)soul;
    return &robo->records[robo->lastSenseKind];
}

// ---- Wander ----

// @004023b0
WanderBehavior::WanderBehavior(SqueezilBody *bodyArg, SqueezilSoul *soulArg) : Behavior(bodyArg, soulArg)
{
    timer = 50;
    phase = 0;
    move = new MoveActivity(bodyArg, soulArg);
    turn = new TurnTargetActivity(bodyArg, soulArg);
    stop = new StopActivity(bodyArg, soulArg);
    vtbl = kWanderVtable;
    move->direction = 1;
    move->turn = 0;
    move->gait = SS_WALK;
    turn->direction = 1;
    turn->turn = 0;
    turn->gait = SS_STAND;
    current = stop;
}

// @004025b0 slot 0
WanderBehavior *WanderBehavior::DestroyAndFree(int flags)
{
    vtbl = kWanderVtable;
    delete move;
    move = NULL;
    delete turn;
    turn = NULL;
    delete stop;
    stop = NULL;
    vtbl = kBehaviorVtable;
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @004026e0 A random corner of a random face of the surface the creature stands on
void WanderBehavior::PickTarget()
{
    Contact contact = *BodyCall::GetContact(body);
    NavMesh *mesh = (NavMesh *)WorldSurface(GetWorld(), contact.face);
    unsigned face = (unsigned)GameRandom() % mesh->faceCount;
    turn->target = mesh->points[mesh->faces[face].vertex[0]];
}

// @004028a0 slot 1 Pick a point and turn to it; walk (fifty frames' worth of pause, then again) once turned. The physics are taken
// anew every frame.
int WanderBehavior::Step(GroundControl *ground, AirControl *air)
{
    // (the original asks the soul for its state here and does nothing with it)
    switch (phase) {
    case 0:
        PickTarget();
        current = turn;
        phase++;
        break;
    case 1:
        if (IsZero(turn->turnAmount)) {
            current = move;
            phase++;
            timer = 50;
        }
        break;
    case 2:
        timer--;
        if (timer < 0)
            phase = 0;
        break;
    }
    current->RefreshPhysics();
    return ActivityStepGround(current, ground);
}

// ---- Pace ----

// @00402930
PaceBehavior::PaceBehavior(SqueezilBody *bodyArg, SqueezilSoul *soulArg, float degrees, float distance, float pause)
    : Behavior(bodyArg, soulArg)
{
    timer = 0;
    phase = 0;
    move = new MoveActivity(bodyArg, soulArg);
    turn = new TurnTargetActivity(bodyArg, soulArg);
    stop = new StopActivity(bodyArg, soulArg);
    vtbl = kPaceVtable;
    turnDegrees = degrees;
    pauseSeconds = pause;
    walkSeconds = distance / soulArg->physics->walk.speed;      // how long the distance takes at a walk
    move->direction = 1;
    move->turn = 0;
    move->gait = SS_WALK;
    turn->direction = 1;
    turn->turn = 0;
    turn->gait = SS_STAND;
    current = stop;
}

// @00402ba0 slot 0
PaceBehavior *PaceBehavior::DestroyAndFree(int flags)
{
    vtbl = kPaceVtable;
    delete move;
    move = NULL;
    delete turn;
    turn = NULL;
    delete stop;
    stop = NULL;
    vtbl = kBehaviorVtable;
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00402cd0 slot 1 Phases: 0 (one frame), 1 stand for the pause, 2 walk the stretch, 3 stand for the pause, 4 turn by the angle (a point
// ten units ahead after the turn is the target), 5 wait for the turn, 6 back to 1. On the ground or in the air, the current activity is
// asked the matching question.
int PaceBehavior::Step(GroundControl *ground, AirControl *air)
{
    switch (phase) {
    case 0:
        phase++;
        break;
    case 1:
        current = stop;
        phase++;
        timer = Milliseconds(pauseSeconds);
        break;
    case 2:
        timer -= FrameMilliseconds();
        if (timer <= 0) {
            current = move;
            phase++;
            timer = Milliseconds(walkSeconds);
        }
        break;
    case 3:
        timer -= FrameMilliseconds();
        if (timer <= 0) {
            current = stop;
            phase++;
            timer = Milliseconds(pauseSeconds);
        }
        break;
    case 4:
        timer -= FrameMilliseconds();
        if (timer <= 0) {
            float m[16], turned[16], rotation[16];
            float *bodyMatrix = BodyCall::Matrix(body);
            for (int i = 0; i < 16; i++)
                m[i] = bodyMatrix[i];
            Vector3f here = Row(m, 3);
            RotationY(rotation, turnDegrees * kRadiansPerDegree);
            MultiplyMatrix(turned, rotation, m);
            Vector3f ahead = Row(turned, 2) * 10.0f;
            current = turn;
            turn->target = here + ahead;
            phase++;
        }
        break;
    case 5:
        if (IsZero(turn->turnAmount))
            phase++;
        break;
    case 6:
        phase = 1;
        break;
    }
    if (BodyCall::IsOnGround(body))
        return ActivityStepGround(current, ground);
    return ActivityStepAir(current, air);
}

// ---- Death ----

// @004030f0
DeathBehavior::DeathBehavior(SqueezilBody *bodyArg, SqueezilSoul *soulArg, float regenAfter) : Behavior(bodyArg, soulArg)
{
    phase = 0;
    die = new DieActivity(bodyArg, soulArg);
    regen = new RegenActivity(bodyArg, soulArg);
    timer = 0;
    vtbl = kDeathVtable;
    regenSeconds = regenAfter;
}

// @00403230 slot 0
DeathBehavior *DeathBehavior::DestroyAndFree(int flags)
{
    vtbl = kDeathVtable;
    delete die;
    die = NULL;
    delete regen;
    regen = NULL;
    vtbl = kBehaviorVtable;
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00403310 slot 1
int DeathBehavior::Step(GroundControl *ground, AirControl *air)
{
    switch (phase) {
    case 0:
        current = die;
        phase++;
        break;
    case 1:
        if (SoulCall::GetState(soul) == SS_DEAD) {
            timer = Milliseconds(regenSeconds);
            phase++;
        }
        break;
    case 2:
        if (regenSeconds > 0.0f) {
            timer -= FrameMilliseconds();
            if (timer <= 0) {
                current = regen;
                phase++;
            }
        }
        break;
    case 3:
        if (SoulCall::GetState(soul) != SS_DEAD)       // it lives again
            phase++;
        break;
    }
    return ActivityStepGround(current, ground);
}

// ---- SimpleFly ----

// @004033b0
SimpleFlyBehavior::SimpleFlyBehavior(SqueezilBody *bodyArg, SqueezilSoul *soulArg) : Behavior(bodyArg, soulArg)
{
    phase = 0;
    fly = new FlyTargetActivity(bodyArg, soulArg);
    vtbl = kSimpleFlyVtable;
    fly->pitch = 0;
    fly->turn = 0;
}

// @004034b0 slot 0
SimpleFlyBehavior *SimpleFlyBehavior::DestroyAndFree(int flags)
{
    vtbl = kSimpleFlyVtable;
    delete fly;
    fly = NULL;
    vtbl = kBehaviorVtable;
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00403580 slot 1 The first time: the target is the creature's own place, 10 high if it is higher than 10 now, else 1. Then it flies.
int SimpleFlyBehavior::Step(GroundControl *ground, AirControl *air)
{
    if (phase == 0) {
        current = fly;
        Vector3f here = Row(BodyCall::Matrix(body), 3);
        fly->target = Vector3f(here.x, (here.y > 10.0f) ? 10.0f : 1.0f, here.z);
        phase++;
    }
    return ActivityStepAir(current, air);
}

// ---- Chase ----

// @00403650
ChaseBehavior::ChaseBehavior(SqueezilBody *bodyArg, SqueezilSoul *soulArg) : Behavior(bodyArg, soulArg)
{
    phase = 0;
    move = new MoveActivity(bodyArg, soulArg);
    turn = new TurnTargetActivity(bodyArg, soulArg);
    stop = new StopActivity(bodyArg, soulArg);
    vtbl = kChaseVtable;
    current = stop;
}

// @00403800 slot 0
ChaseBehavior *ChaseBehavior::DestroyAndFree(int flags)
{
    vtbl = kChaseVtable;
    delete turn;
    turn = NULL;
    delete stop;
    stop = NULL;
    delete move;
    move = NULL;
    vtbl = kBehaviorVtable;
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00403930 slot 1 The target is where the creature will be in as long as it takes to get to where it is (two seconds at the most).
// The turn is walked, then run once it is done; if the target gets more than 50 degrees off the run goes back to a walk.
int ChaseBehavior::Step(GroundControl *ground, AirControl *air)
{
    AwarenessRecord *record = Noticed();
    SqueezilBody *quarry = (SqueezilBody *)SimulationActor(record->id);
    Vector3f velocity = BodyCall::Direction(quarry) * BodyCall::Speed(quarry);
    float seconds = sqrtf(record->distance) / BodyCall::Speed(body);
    if (seconds > kChaseLookAhead)
        seconds = kChaseLookAhead;
    turn->target = ToVector(record->position) + velocity * seconds;

    switch (phase) {
    case 0:
        current = turn;
        turn->gait = SS_WALK;
        phase++;
        break;
    case 1:
        if (IsZero(turn->turnAmount)) {
            turn->gait = SS_RUN;
            phase++;
        }
        break;
    case 2:
        if (turn->AngleToTarget() > kChaseRunAngle) {
            turn->gait = SS_WALK;
            phase--;
        }
        break;
    }
    return ActivityStepGround(current, ground);
}

// ---- Spit ----

// @00403b60
SpitBehavior::SpitBehavior(SqueezilBody *bodyArg, SqueezilSoul *soulArg, float range, float pause) : Behavior(bodyArg, soulArg)
{
    phase = 0;
    turn = new TurnTargetActivity(bodyArg, soulArg);
    stop = new StopActivity(bodyArg, soulArg);
    timer = 0;
    rangeSquared = range * range;
    vtbl = kSpitVtable;
    pauseSeconds = pause;
    current = stop;
}

// @00403c90 slot 0
SpitBehavior *SpitBehavior::DestroyAndFree(int flags)
{
    vtbl = kSpitVtable;
    delete turn;
    turn = NULL;
    delete stop;
    stop = NULL;
    vtbl = kBehaviorVtable;
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00403d90 slot 1 Turn to the creature (standing), wait until it is near enough and a nut is carried, start the spit, let it go
// when the turn is done and pause. The state the spit asks for is the answer while it lasts.
int SpitBehavior::Step(GroundControl *ground, AirControl *air)
{
    AwarenessRecord *record = Noticed();
    turn->target = ToVector(record->position);
    int wanted = SS_NONE;
    switch (phase) {
    case 0:
        current = turn;
        turn->gait = SS_STAND;
        phase++;
        break;
    case 1:
        if (record->distance < rangeSquared)
            phase++;
        break;
    case 2:
        if (InventoryCount(SoulCall::GetInventory(soul), kSpitNutGoodie) > 0) {
            wanted = CallVirtual<int>(soul, SQS_SPIT);
            phase++;
        }
        break;
    case 3:
        if (IsZero(turn->turnAmount)) {
            CallVirtual<void>(soul, SQS_SET_SPIT_STRENGTH, 1.0f);
            timer = Milliseconds(pauseSeconds);
            phase++;
        }
        break;
    case 4:
        timer -= FrameMilliseconds();
        if (timer <= 0)
            phase = 0;
        break;
    }
    int result = ActivityStepGround(current, ground);
    return (wanted != SS_NONE) ? wanted : result;
}

// ---- Flee ----

// @00403ed0
FleeBehavior::FleeBehavior(SqueezilBody *bodyArg, SqueezilSoul *soulArg) : Behavior(bodyArg, soulArg)
{
    phase = 0;
    move = new MoveActivity(bodyArg, soulArg);
    turn = new TurnTargetActivity(bodyArg, soulArg);
    counter = 0;
    vtbl = kFleeVtable;
    current = move;
}

// @00404020 slot 0
FleeBehavior *FleeBehavior::DestroyAndFree(int flags)
{
    vtbl = kFleeVtable;
    delete move;
    move = NULL;
    delete turn;
    turn = NULL;
    vtbl = kBehaviorVtable;
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @00404120 slot 1 Decides every frame once the ten frames' wait is over: 0 back off (walking backwards) unless tumbling or sliding;
// 1 turn to a point ten times the way from the creature to here, past here; 2 while the creature moves aim again, when it is
// still and the turn is done go on; 3 walk forwards there.
int FleeBehavior::Step(GroundControl *ground, AirControl *air)
{
    AwarenessRecord *record = Noticed();
    counter--;
    if (counter <= 0) {
        switch (phase) {
        case 0: {
            int state = SoulCall::GetState(soul);
            if (state == SS_STUMBLE || SoulCall::GetState(soul) == SS_SLIP)
                break;
            current = move;
            move->gait = SS_WALK;
            move->direction = -1;
            counter = 10;
            phase++;
            break;
        }
        case 1: {
            current = turn;
            turn->gait = SS_WALK;
            Vector3f here = BodyCall::Position(body);
            Vector3f away = ToVector(record->position) - here;
            turn->target = here + away * -10.0f;
            phase++;
            break;
        }
        case 2: {
            SqueezilBody *quarry = (SqueezilBody *)SimulationActor(record->id);
            if (fabs((double)BodyCall::Speed(quarry)) > 0.01f)
                phase--;
            else if (IsZero(turn->turnAmount))
                phase++;
            break;
        }
        case 3:
            current = move;
            move->direction = 1;
            move->gait = SS_WALK;
            phase++;
            break;
        }
    }
    return ActivityStepGround(current, ground);
}
