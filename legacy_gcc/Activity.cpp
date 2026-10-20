// Activity: see Activity.h. Reconstructed from SQZ.EXE 0x401000-0x402250.
#include "Activity.h"
#include "SqueezilSoulInternal.h"
#include "SqueezilRoboSoul.h"
#include "PlaceSimulation.h"
#include "NavMesh.h"
#include "IniReader.h"
#include "CreatureTemplate.h"
#include "Original.h"
#include <math.h>

static_assert(sizeof(SqueezilPhysics) == 0x2a0, "SqueezilPhysics layout");

static void **const kActivityVtable   = (void **)0x49d004;
static void **const kTurnTargetVtable = (void **)0x49d06c;
static void **const kMoveVtable       = (void **)0x49d07c;
static void **const kDieVtable        = (void **)0x49d09c;
static void **const kRegenVtable      = (void **)0x49d0ac;
static void **const kStopVtable       = (void **)0x49d0dc;
static void **const kFlyTargetVtable  = (void **)0x49d11c;

const float kStillSpeed = 0.01f;            // 0x49d014: the body moves slower than this and it counts as standing still

// The turning settings of [RoboPhysics], read when the first TurnTargetActivity is made (the game's own globals: the original's
// TurnTargetActivity reads the same ones)
static float &gSlowingCutoff = *(float *)0x4c63c0;      // TurnTargetSlowingCutoff (0.7): off the target by (1 - cos) more than this and it turns at full rate
static float &gSlowingFactor = *(float *)0x4c63c4;      // TurnTargetSlowingFactor (0.5)
static float &gMinSlowing    = *(float *)0x4c63c8;      // TurnTargetMinSlowing (0.035): the least turn rate while turning at all
static int   &gSlowingRead   = *(int *)0x4b90e0;

// @00401000 The physics are a copy of the soul's, taken now
Activity::Activity(SqueezilBody *bodyArg, SqueezilSoul *soulArg)
{
    vtbl = kActivityVtable;
    body = bodyArg;
    soul = soulArg;
    physics = *soulArg->physics;
}

// @00401270 (the original copies it piece by piece)
void Activity::RefreshPhysics()
{
    physics = *soul->physics;
}

MoveActivity::MoveActivity(SqueezilBody *bodyArg, SqueezilSoul *soulArg) : Activity(bodyArg, soulArg)
{
    vtbl = kMoveVtable;
    direction = 1;
    turn = 0;
    turnAmount = 0.0f;
    gait = SS_WALK;
}

// @00401430 slot 1 What the gait needs: stand (when moving), walk (when still), run (always). The turn is the turn wish times the
// share of the turn rate to use.
int MoveActivity::StepGround(GroundControl *out)
{
    int state = SoulCall::GetState(soul);
    int wanted = SS_NONE;
    switch (gait) {
    case SS_STAND:
        out->forward = 0.0f;
        if (state != SS_STAND && BodyCall::Speed(body) <= kStillSpeed)
            wanted = SS_STAND;
        break;
    case SS_WALK:
        out->forward = (float)direction;
        if (state != SS_WALK && BodyCall::Speed(body) > kStillSpeed)
            wanted = SS_WALK;
        break;
    case SS_RUN:
    case SS_ATTACK:
        out->forward = 1.0f;
        if (state != gait)
            wanted = gait;
        break;
    default:            // the other gaits give no forward wish and no state
        break;
    }
    out->turn = (float)turn * turnAmount;
    return wanted;
}

// @00401510
TurnTargetActivity::TurnTargetActivity(SqueezilBody *bodyArg, SqueezilSoul *soulArg) : MoveActivity(bodyArg, soulArg)
{
    vtbl = kTurnTargetVtable;
    target = Vector3f(0.0f, 0.0f, 0.0f);        // (the original leaves target and cosAngle as the memory had them)
    cosAngle = 1.0f;
    if (!gSlowingRead) {
        IniReader ini;
        ini.SetSection("RoboPhysics");
        gSlowingCutoff = ini.GetFloat("TurnTargetSlowingCutoff", 0.7f);
        gSlowingFactor = ini.GetFloat("TurnTargetSlowingFactor", 0.5f);
        gMinSlowing    = ini.GetFloat("TurnTargetMinSlowing", 0.035f);
        gSlowingRead = 1;
    }
}

// @004016e0
float TurnTargetActivity::AngleToTarget() const
{
    return (float)acos((double)cosAngle) * kDegreesPerRadian;
}

// @00401700 slot 1 Turns towards the target point, walking as the MoveActivity does meanwhile. The turn wish is the side the target is
// on (seen along the surface the squeezil stands on) and the share of the turn rate grows with how far round the target is; when it
// is nearly dead ahead the squeezil is simply turned the rest of the way.
int TurnTargetActivity::StepGround(GroundControl *out)
{
    Vector3f toTarget = target - Row(BodyCall::Matrix(body), 3);
    Normalize(toTarget);

    Contact *contact = BodyCall::GetContact(body);
    NavMesh *mesh = (NavMesh *)WorldSurface(GetWorld(), contact->face);
    Vector3f normal = mesh->faces[contact->second].normal;
    Vector3f along = toTarget - normal * Dot(normal, toTarget);         // the way to the target along the surface
    Normalize(along);

    float cosine = Dot(Row(BodyCall::Matrix(body), 2), along);          // against the way the squeezil faces
    float off = 1.0f - cosine;
    cosAngle = cosine;

    if (fabsf(off) < 0.0001f) {
        turn = 0;
        turnAmount = 0.0f;
        if (cosine != 1.0f) {                                           // dead ahead but for a hair: turn that hair now
            float angle = (cosine > 1.0f) ? -(float)acos((double)(2.0f - cosine)) : (float)acos((double)cosine);
            CallVirtual<void>(soul, SQS_TURN, angle);                   // (always to the same side, whichever way the target is)
        }
    } else {
        turn = (Dot(Row(BodyCall::Matrix(body), 0), along) < 0.0f) ? -1 : 1;
        if (gSlowingCutoff * 0.5f > off)
            turnAmount = off * off * gSlowingFactor + gMinSlowing;
        else if (off < gSlowingCutoff)
            turnAmount = off * gSlowingFactor * 2.0f + gMinSlowing;
        else
            turnAmount = 1.0f;
    }
    return MoveActivity::StepGround(out);
}

StopActivity::StopActivity(SqueezilBody *bodyArg, SqueezilSoul *soulArg) : Activity(bodyArg, soulArg)
{
    vtbl = kStopVtable;
}

// @00401be0 slot 1 No wish; stand if it is not standing and has come to rest
int StopActivity::StepGround(GroundControl *out)
{
    out->turn = 0.0f;
    out->forward = 0.0f;
    int wanted = SS_NONE;
    if (SoulCall::GetState(soul) != SS_STAND && BodyCall::Speed(body) <= kStillSpeed)
        wanted = SS_STAND;
    return wanted;
}

DieActivity::DieActivity(SqueezilBody *bodyArg, SqueezilSoul *soulArg) : Activity(bodyArg, soulArg)
{
    vtbl = kDieVtable;
    done = 0;
}

// @00401ef0 slot 1 The first time: the level is told which goodies the creature gave up (so that they are not made again) and the
// nuts it carried are taken away. Then (and ever after) the state is SS_DEAD.
int DieActivity::StepGround(GroundControl *out)
{
    if (!done) {
        Inventory *inventory = SoulCall::GetInventory(soul);
        PlaceLevelForgetInventory((PlaceLevel *)GetPlaceGoodies(soul->place), inventory);
        InventoryRemoveNuts(SoulCall::GetInventory(soul));
        done = 1;
    }
    return SS_DEAD;
}

RegenActivity::RegenActivity(SqueezilBody *bodyArg, SqueezilSoul *soulArg) : Activity(bodyArg, soulArg)
{
    vtbl = kRegenVtable;
}

// @00401fd0 slot 1 Back where the level file put the creature, not moving, and the template's function for state 0 run; stand
int RegenActivity::StepGround(GroundControl *out)
{
    SqueezilRoboSoul *robo = (SqueezilRoboSoul *)soul;
    // (the original also looks the soul up in its place, @004140b0, and uses nothing of the answer)
    PlaceLevel *level = (PlaceLevel *)GetPlaceGoodies(robo->place);
    BodyPlacement start = *(BodyPlacement *)level->creatures.At(robo->levelIndex);
    ((SqueezilBody *)body)->Place(start);
    SoulCall::ResetSurface(robo);
    BodyCall::SetVelocity(body, BodyCall::Direction(body), 0.0f);           // stands where it is placed
    robo->character->TrySetState(0, robo);
    robo->character->RunPending(robo);
    return SS_STAND;
}

FlyTargetActivity::FlyTargetActivity(SqueezilBody *bodyArg, SqueezilSoul *soulArg) : FlyActivity(bodyArg, soulArg)
{
    vtbl = kFlyTargetVtable;
    target = Vector3f(0.0f, 0.0f, 0.0f);
}

// @00401c60 slot 0 Climb or dive to the height of the target and turn to it: only the wishes are given, the state stays
int FlyTargetActivity::StepAir(AirControl *out)
{
    float *m = BodyCall::Matrix(body);
    pitch = (m[13] - target.y > 0.0f) ? 1 : -1;                         // above the target's height: dive

    Vector3f toTarget = target - Row(BodyCall::Matrix(body), 3);
    if (Length(toTarget) < 1.0f) {
        turn = 0;
    } else {
        Vector3f up = Row(BodyCall::Matrix(body), 1);
        Vector3f flat = toTarget - up * Dot(up, toTarget);              // the way to the target without the height
        Normalize(flat);
        turn = (Dot(Row(BodyCall::Matrix(body), 0), flat) < 0.0f) ? -1 : 1;
    }
    out->pitch = (float)pitch;                                          // (@00401c30, the FlyActivity's answer)
    out->turn = (float)turn;
    return SS_NONE;
}
