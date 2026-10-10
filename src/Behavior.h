// Behavior: what a robo squeezil (SqueezilRoboSoul) does with its time. Reconstructed from SQZ.EXE 0x4022f0-0x404380 (RTTI classes Behavior,
// WanderBehavior, PaceBehavior, DeathBehavior, SimpleFlyBehavior, ChaseBehavior, SpitBehavior and FleeBehavior; the vtables are 0x49d0bc,
// 0x49d0cc, 0x49d0ec, 0x49d0fc, 0x49d10c, 0x49d13c, 0x49d14c and 0x49d15c).
//
// A behaviour owns some activities (Activity.h) and a little state machine (phase). Every frame the soul asks it for the controls
// (slot 1: the ground or the air control record, one of them NULL) and the answer is the state to go to. The behaviour picks which of
// its activities is the current one as its phase moves on, gives it what it needs to know (a point to walk to) and passes the question
// on to it. The class number the soul keeps (SqueezilRoboSoul::mode) is: 0 Pace, 1 Wander, 2 SimpleFly, 3 Chase, 4 Spit, 5 Flee, 6 Death.
//
// The objects have the original's tables (slot 0 destroys, slot 1 asks) and layouts and are on the game's heap.
#pragma once
#include "Activity.h"
#include "SqueezilRoboSoul.h"

struct Behavior {
    GAME_HEAP_OPERATORS
    void         **vtbl;        // 0x00
    SqueezilBody  *body;        // 0x04
    SqueezilSoul  *soul;        // 0x08
    Activity      *current;     // 0x0c the activity asked

    Behavior(SqueezilBody *body, SqueezilSoul *soul);   // @004022f0 (ret 8)
    // The soul's record of the creature it noticed last
    AwarenessRecord *Noticed() const;
};

// Walks to a random corner of the walkable surface it stands on, stops, and goes again. 0x24 bytes.
struct WanderBehavior : Behavior {
    int                 timer;      // 0x10 milliseconds of the pause left
    int                 phase;      // 0x14 0 pick a point and turn to it, 1 turn, 2 walk (the pause counts down, and then it starts again)
    MoveActivity       *move;       // 0x18
    TurnTargetActivity *turn;       // 0x1c
    StopActivity       *stop;       // 0x20

    WanderBehavior(SqueezilBody *body, SqueezilSoul *soul);                 // @004023b0 (ret 8)
    WanderBehavior *DestroyAndFree(int flags);                              // @004025b0 slot 0
    int  Step(GroundControl *ground, AirControl *air);                      // @004028a0 slot 1
    void PickTarget();                                                      // @004026e0
};
static_assert(sizeof(WanderBehavior) == 0x24, "WanderBehavior layout");

// Walks a stretch, stops, turns by a fixed angle and goes on: a patrol. 0x30 bytes.
struct PaceBehavior : Behavior {
    int                 timer;      // 0x10 milliseconds left of the walk or the pause
    int                 phase;      // 0x14
    MoveActivity       *move;       // 0x18
    TurnTargetActivity *turn;       // 0x1c
    StopActivity       *stop;       // 0x20
    float               turnDegrees;    // 0x24 how far round it turns at the end of a stretch
    float               walkSeconds;    // 0x28 how long a stretch takes
    float               pauseSeconds;   // 0x2c how long it stands still between

    PaceBehavior(SqueezilBody *body, SqueezilSoul *soul, float turnDegrees, float distance, float pauseSeconds);   // @00402930 (ret 0x14)
    PaceBehavior *DestroyAndFree(int flags);                                // @00402ba0 slot 0
    int  Step(GroundControl *ground, AirControl *air);                      // @00402cd0 slot 1
};
static_assert(sizeof(PaceBehavior) == 0x30, "PaceBehavior layout");

// Dies, and (if it is given a time) comes back to life later. 0x24 bytes.
struct DeathBehavior : Behavior {
    int            phase;           // 0x10 0 start dying, 1 wait for SS_DEAD, 2 count down to the return, 3 wait for the life to return
    DieActivity   *die;             // 0x14
    RegenActivity *regen;           // 0x18
    float          regenSeconds;    // 0x1c how long it stays dead (0 or less: for good)
    int            timer;           // 0x20 milliseconds left

    DeathBehavior(SqueezilBody *body, SqueezilSoul *soul, float regenSeconds);   // @004030f0 (ret 0xc)
    DeathBehavior *DestroyAndFree(int flags);                               // @00403230 slot 0
    int  Step(GroundControl *ground, AirControl *air);                      // @00403310 slot 1
};
static_assert(sizeof(DeathBehavior) == 0x24, "DeathBehavior layout");

// Flies at a height of about 10 (or 1 near the ground) above where it first was. 0x18 bytes.
struct SimpleFlyBehavior : Behavior {
    int                phase;       // 0x10
    FlyTargetActivity *fly;         // 0x14

    SimpleFlyBehavior(SqueezilBody *body, SqueezilSoul *soul);              // @004033b0 (ret 8)
    SimpleFlyBehavior *DestroyAndFree(int flags);                           // @004034b0 slot 0
    int  Step(GroundControl *ground, AirControl *air);                      // @00403580 slot 1
};
static_assert(sizeof(SimpleFlyBehavior) == 0x18, "SimpleFlyBehavior layout");

// Runs after the creature it noticed, aiming where it will be. 0x20 bytes.
struct ChaseBehavior : Behavior {
    int                 phase;      // 0x10 0 turn (walking), 1 run once turned, 2 walk again when it is turned too far away
    MoveActivity       *move;       // 0x14 (not used)
    TurnTargetActivity *turn;       // 0x18
    StopActivity       *stop;       // 0x1c

    ChaseBehavior(SqueezilBody *body, SqueezilSoul *soul);                  // @00403650 (ret 8)
    ChaseBehavior *DestroyAndFree(int flags);                               // @00403800 slot 0
    int  Step(GroundControl *ground, AirControl *air);                      // @00403930 slot 1
};
static_assert(sizeof(ChaseBehavior) == 0x20, "ChaseBehavior layout");

// Faces the creature it noticed, and when it is near enough and has a nut, spits. 0x28 bytes.
struct SpitBehavior : Behavior {
    int                 phase;      // 0x10 0 turn, 1 wait for it to come near, 2 wait for a nut and start the spit, 3 let it go once turned, 4 pause
    TurnTargetActivity *turn;       // 0x14
    StopActivity       *stop;       // 0x18
    int                 timer;      // 0x1c milliseconds of the pause left
    float               rangeSquared;   // 0x20 how near it must be (squared)
    float               pauseSeconds;   // 0x24

    SpitBehavior(SqueezilBody *body, SqueezilSoul *soul, float range, float pauseSeconds);   // @00403b60 (ret 0x10)
    SpitBehavior *DestroyAndFree(int flags);                                // @00403c90 slot 0
    int  Step(GroundControl *ground, AirControl *air);                      // @00403d90 slot 1
};
static_assert(sizeof(SpitBehavior) == 0x28, "SpitBehavior layout");

// Walks away backwards for a moment, then turns to a point far from the creature it noticed and walks there. 0x20 bytes.
struct FleeBehavior : Behavior {
    int                 phase;      // 0x10
    MoveActivity       *move;       // 0x14
    TurnTargetActivity *turn;       // 0x18
    int                 counter;    // 0x1c frames to wait between decisions

    FleeBehavior(SqueezilBody *body, SqueezilSoul *soul);                   // @00403ed0 (ret 8)
    FleeBehavior *DestroyAndFree(int flags);                                // @00404020 slot 0
    int  Step(GroundControl *ground, AirControl *air);                      // @00404120 slot 1
};
static_assert(sizeof(FleeBehavior) == 0x20, "FleeBehavior layout");
