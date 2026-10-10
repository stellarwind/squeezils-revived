// Activity: the small objects a robo squeezil's behaviour (Behavior.h) hands the controls to. Reconstructed from SQZ.EXE
// 0x401000-0x402250 and 0x405200-0x405380 (RTTI classes Activity, WalkActivity, MoveActivity, TurnTargetActivity, StopActivity,
// DieActivity, RegenActivity, FlyActivity and FlyTargetActivity; the vtables are 0x49d004 and 0x49d06c-0x49d0dc, 0x49d11c, 0x49d12c).
//
// An activity is asked, every frame, what a player would do with the keys: the ground control record (GroundControl: forward and
// turn) or the air control record (AirControl: pitch and turn) is filled in and the answer is the state the squeezil should go to
// (SS_NONE: none). Slot 0 of an activity's table is the air question, slot 1 the ground question; a class that only knows one of them
// has the original's stub for the other (@00405200 for the walkers, @00405370 for the flyers: both answer SS_NONE and touch nothing).
//
// Every activity starts with a copy of the soul's physics (what the soul can do: how fast it walks and turns), taken when it is made;
// the Wander behaviour takes a fresh copy every frame (RefreshPhysics).
//
// The objects are made and used by the behaviours (and, when only some of the replacements are on, by the original's behaviours), so
// they keep the original's tables and layouts. They are on the game's heap.
#pragma once
#include "SqueezilSoul.h"
#include "Vector3f.h"
#include "Container.h"
#include "VirtualCall.h"

struct SqueezilBody;

// The base class (0x2ac bytes)
struct Activity {
    GAME_HEAP_OPERATORS
    void           **vtbl;          // 0x00
    SqueezilBody    *body;          // 0x04
    SqueezilSoul    *soul;          // 0x08
    SqueezilPhysics  physics;       // 0x0c a copy of the soul's (0x2a0 bytes)

    Activity(SqueezilBody *body, SqueezilSoul *soul);       // @00401000 (ret 8)
    void RefreshPhysics();                                  // @00401270 take the soul's physics again
};
static_assert(sizeof(Activity) == 0x2ac, "Activity layout");

// Walking, and standing still (the gait is the SqueezilState to go to: SS_STAND, SS_WALK, SS_RUN). 0x2bc bytes.
struct MoveActivity : Activity {
    int   direction;        // 0x2ac 1 forward, -1 backward (the forward wish; only a walk uses it, a run is always 1)
    int   turn;             // 0x2b0 -1, 0 or 1: which way to turn
    float turnAmount;       // 0x2b4 how much of the turn rate to use (0..1)
    int   gait;             // 0x2b8 the state wanted: SS_STAND, SS_WALK or SS_RUN (SS_ATTACK is answered like a run)

    MoveActivity(SqueezilBody *body, SqueezilSoul *soul);   // starts walking forward without turning (the original builds these inline)
    // @00401430 slot 1
    int StepGround(GroundControl *out);
};
static_assert(sizeof(MoveActivity) == 0x2bc, "MoveActivity layout");

// Walks (as a MoveActivity does) and turns on the spot towards a point first. 0x2cc bytes.
struct TurnTargetActivity : MoveActivity {
    Vector3f target;        // 0x2bc the point to face
    float    cosAngle;      // 0x2c8 the cosine of the angle between where the squeezil faces and the target (the last StepGround)

    TurnTargetActivity(SqueezilBody *body, SqueezilSoul *soul);   // @00401510 (ret 8) also reads the [RoboPhysics] turning settings once
    // @00401700 slot 1
    int   StepGround(GroundControl *out);
    // @004016e0 how many degrees off the target the squeezil faced
    float AngleToTarget() const;
};
static_assert(sizeof(TurnTargetActivity) == 0x2cc, "TurnTargetActivity layout");

// Stands still. 0x2ac bytes.
struct StopActivity : Activity {
    StopActivity(SqueezilBody *body, SqueezilSoul *soul);
    // @00401be0 slot 1
    int StepGround(GroundControl *out);
};
static_assert(sizeof(StopActivity) == 0x2ac, "StopActivity layout");

// Puts the creature out of the game: the dead robo's goodies are noted in the level and its nuts taken away, once. 0x2b0 bytes.
struct DieActivity : Activity {
    int done;               // 0x2ac

    DieActivity(SqueezilBody *body, SqueezilSoul *soul);    // @00401e60 (ret 8)
    // @00401ef0 slot 1: answers SS_DEAD
    int StepGround(GroundControl *out);
};
static_assert(sizeof(DieActivity) == 0x2b0, "DieActivity layout");

// Brings the creature back: it is put where the level file had it and its Character's state 0 function is run. 0x2ac bytes.
struct RegenActivity : Activity {
    RegenActivity(SqueezilBody *body, SqueezilSoul *soul);  // @00401f50 (ret 8)
    // @00401fd0 slot 1: answers SS_STAND
    int StepGround(GroundControl *out);
};
static_assert(sizeof(RegenActivity) == 0x2ac, "RegenActivity layout");

// Flying towards a point: climbs or dives to its height and turns towards it. 0x2c0 bytes (FlyActivity, with the two wishes, is 0x2b4).
struct FlyActivity : Activity {
    int   pitch;            // 0x2ac -1 or 1: the pitch wish
    int   turn;             // 0x2b0 -1, 0 or 1: which way to turn

    FlyActivity(SqueezilBody *body, SqueezilSoul *soul) : Activity(body, soul), pitch(0), turn(0) {}
};
static_assert(sizeof(FlyActivity) == 0x2b4, "FlyActivity layout");

struct FlyTargetActivity : FlyActivity {
    Vector3f target;        // 0x2b4

    FlyTargetActivity(SqueezilBody *body, SqueezilSoul *soul);
    // @00401c60 slot 0
    int StepAir(AirControl *out);
};
static_assert(sizeof(FlyTargetActivity) == 0x2c0, "FlyTargetActivity layout");

// The activities' slots (the stubs of the classes that do not ask the other question are the original's)
enum ActivitySlot { ACTIVITY_STEP_AIR = 0, ACTIVITY_STEP_GROUND = 1 };
inline int ActivityStepGround(Activity *a, GroundControl *out) { return CallVirtual<int>(a, ACTIVITY_STEP_GROUND, out); }   // through the table
inline int ActivityStepAir(Activity *a, AirControl *out)       { return CallVirtual<int>(a, ACTIVITY_STEP_AIR, out); }
