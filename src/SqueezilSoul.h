// SqueezilSoul: the part of a squeezil that decides how it moves (walking, running, scampering, flying, diving, jumping, hopping,
// slipping, attacking, spitting nuts). Reconstructed from SQZ.EXE 0x44f1a0-0x456f40: the constructor, the destructor, the
// physics settings it reads from the ini and the 60 virtual functions (the movement code is in SqueezilSoulMove.cpp).
//
// How it works. A squeezil is in one of 23 states (SqueezilState). Every frame Update (slot 5) asks the subclass for the
// player's or the AI's wishes (slots 39 and 40, which SqueezilSoul leaves to SqueezilPlayerSoul / SqueezilRoboSoul / the region
// soul), then hands them to the handler of the current state (slots 43-59). A handler moves the body (SqueezilBody) for one
// frame and may switch state; switching (SetState) also picks the body's animation clip and whether it counts as on the ground
// from two tables, and a third table says which state may follow which. On the ground the squeezil is placed on a surface face
// of the place (the body's Contact) and moved along it; in the air a small flight model with drag and lift is integrated.
//
// Every squeezil soul owns a copy of the settings (its +0x48); the copy is taken from the shared defaults at 0x4c7828, which the
// first soul made reads from the [Physics] section. The block starts with the settings every physical soul shares (gravity and
// friction, read by PhysicalSoul at 0x446ed0).
//
// Like the bodies, the object is made and used by original code through its vtables (VirtualCall.h): the virtual functions are
// plain members here and the soul calls its own virtual functions through the table, so the subclasses' versions run.
#pragma once
#include "Vector3f.h"
#include "Float3.h"
#include "GameString.h"

struct Contact;
struct SqueezilBody;

// PhysicalSoul's settings (0x24 bytes; the shared copy is at 0x4c7768, read from [Physics] Gravity / StaticFriction /
// KineticFriction by 0x446ed0).
struct PhysicalSettings {
    Vector3f gravity;           // 0x00 (0, Gravity, 0); Gravity defaults to -32
    Vector3f down;              // 0x0c (0, +-1, 0): the direction gravity pulls
    float    gravityStrength;   // 0x18 |Gravity|
    float    kineticFriction;   // 0x1c default 0.6
    float    staticFriction;    // 0x20 default 1

    // @00446dd0 (PhysicalSoul's) gravity (0, g, 0), pulling down (0, sign of g, 0) with strength |g|; returns g
    float SetGravity(float g);
};
static_assert(sizeof(PhysicalSettings) == 0x24, "PhysicalSettings layout");

// One way of moving on the ground (6 floats). The ini gives the decelerations and turn decelerations as negative numbers; they
// are stored as their size (every value here is made positive).
struct GroundMotion {
    float acceleration, deceleration, speed;
    float turnAccel, turnDecel, turnRate;
};
static_assert(sizeof(GroundMotion) == 0x18, "GroundMotion layout");

// Flying (23 floats): steering, then the aerodynamics (drag along each axis and lift, each "mult * speed^exp" judging by the
// names; the formulas are in the soul's flight code, not reconstructed).
struct FlightModel {
    float diveAngle, climbAngle, diveAdjustRate;     // 0x00
    float turnAccel, turnDecel, turnRate;            // 0x0c
    float bank, pitchCorrectionRate;                 // 0x18
    float controlCutoff, speedFactor;                // 0x20
    float dragZExp, dragZMult, dragYExp, dragYMult, dragXExp, dragXMult;   // 0x28
    float liftExp, liftMult;                         // 0x40
    float aoaAdjustMult, stallAdjustMult;            // 0x48
    float tuckLiftFactor, climbLiftFactor, climbPitchFactor;   // 0x50
};
static_assert(sizeof(FlightModel) == 0x5c, "FlightModel layout");

// A jump, a hop or a ski-hop (10 floats)
struct JumpModel {
    float upSpeed, fwdSpeed, upDistance;
    float maxLinearSpeed, linearAccel;
    float dragXMult, dragYMult, dragZMult, aoaAdjustMult;
    float gravityFactor;
};
static_assert(sizeof(JumpModel) == 0x28, "JumpModel layout");

// Slipping and attacking share one set of 11 floats ("lever": the push that turns the squeezil, judging by the names)
struct LeverMotion {
    float speedCutoff, distanceCutoff;               // 0x00
    float turnAccel, turnDecel, turnRate;            // 0x08
    float minSpeed, pushIncrement, frictionScale;    // 0x14 slipping: always 1, 0, 1
    float controlSpeed, leverArm, leverWeightFactor; // 0x20
};
static_assert(sizeof(LeverMotion) == 0x2c, "LeverMotion layout");

struct SqueezilPhysics {
    PhysicalSettings physical;      // 0x000 a copy of PhysicalSoul's
    GroundMotion walk;              // 0x024
    GroundMotion run;               // 0x03c
    GroundMotion scamper;           // 0x054
    GroundMotion skid;              // 0x06c walk's, with SkidDeceleration
    FlightModel  fly;               // 0x084
    FlightModel  dive;              // 0x0e0 fly's, with its own drag and lift and no stall adjustment
    FlightModel  tumble;            // 0x13c dive's, with no angle-of-attack adjustment either: tumbling, and the base of the
                                    //       jumps' flight (JumpFlight puts a jump's drag over it)
    JumpModel    jump;              // 0x198
    JumpModel    hop;               // 0x1c0
    JumpModel    skihop;            // 0x1e8 defaults: jump's speeds and distances
    LeverMotion  attack;            // 0x210 slip's, with its own steering and push
    LeverMotion  slip;              // 0x23c
    float clawGrip;                 // 0x268
    float crouchGrip;               // 0x26c
    float maxLandingSpeed;          // 0x270
    float tumbleDuration;           // 0x274
    float spitVelocity;             // 0x278
    float walkToScamperAngle;       // 0x27c
    float scamperToWalkAngle;       // 0x280
    float cornerAngle;              // 0x284
    int   unknown288;               // 0x288 always 1
    float fwdTapFactor;             // 0x28c
    float rotTapFactor;             // 0x290
    float maxWalkSpeed;             // 0x294
    float minStumbleSpeed;          // 0x298
    float maxRunSpeed;              // 0x29c
};
static_assert(sizeof(SqueezilPhysics) == 0x2a0, "SqueezilPhysics layout");

// @0044f700 Fills the shared defaults (the block the pointer at 0x4c7ad0 points to) from [Physics]. Also sets the two nut spray
// percentages (globals 0x4c7820, 0x4c7b0c). Called by the first SqueezilSoul made.
void ReadSqueezilPhysics();

// ---- the soul ----

// The states, in the order of the original's state tables (0x49ed40, 0x49eda0, 0x49f5e8). The names come from the clip each state
// plays and from what its handler does; the ones marked "guessed" are less certain.
enum SqueezilState {
    SS_STAND,           // 0  standing still (clip stop)
    SS_LIE_DOWN,        // 1  going to sleep, then sleeping (sleep pieces 1 and 2)
    SS_GET_UP,          // 2  waking up (sleep piece 3)
    SS_WALK,            // 3
    SS_RUN,             // 4  (running backwards plays the walk clip)
    SS_SKID,            // 5  braking hard (skid pieces)
    SS_SLIP,            // 6  sliding down a slope out of control (skid clip)
    SS_ATTACK,          // 7  the sliding attack (slide pieces)
    SS_STUMBLE,         // 8  knocked over on the ground, sliding (stumble pieces)
    SS_SCAMPER,         // 9  on all fours, up steep surfaces
    SS_SCAMPER_STOP,    // 10
    SS_TUMBLE,          // 11 knocked into the air, out of control for TumbleDuration
    SS_JUMP,            // 12 in the air after a jump
    SS_HOP_START,       // 13 crouching for a hop, and landing from one
    SS_HOP,             // 14 in the air after a hop (also: leaving a slope at the end of a slip)
    SS_SKIHOP,          // 15 in the air after a ski-hop (guessed: nothing in this class starts it)
    SS_SPIT,            // 16 spitting a nut while standing
    SS_FLY,             // 17 gliding
    SS_DIVE,            // 18 diving (cannon clip)
    SS_LAND,            // 19 just touched down; the next ground state is picked from the speed
    SS_DYING_IN_AIR,    // 20
    SS_DYING,           // 21 dying on the ground (the death clip plays)
    SS_DEAD,            // 22 gone (the body has no clip)
    SS_COUNT,
    SS_NONE = 24        // "no change" for the wished-for next state
};

// What the subclass's input functions fill in for the state handlers, and the settings of the current state (slots 41 and 42 copy
// them from the physics block). One shared record for the ground (global 0x4c7ad8) and one for the air (global 0x4c7b10).
const float kNoInput = 1000.0f;     // "no wish" for forward / pitch (and the soul's pitch target)

struct GroundControl {
    float forward;              // 0x00 -1 (back) .. 1 (forward), times the motion's speed
    float turn;                 // 0x04 -1 .. 1, times the motion's turn rate
    union {
        GroundMotion ground;    // 0x08 walking, running, scampering, skidding
        LeverMotion  lever;     //      slipping, attacking, stumbling
    };
};
static_assert(sizeof(GroundControl) == 0x34, "GroundControl layout");

struct AirControl {
    float pitch;                // 0x00 -1 .. 1 (kNoInput: none); in a jump, the wish to move forward
    float turn;                 // 0x04
    union {
        FlightModel flight;     // 0x08
        JumpModel   jump;       //      replaced by the tumble model with the jump's drag (JumpFlight)
    };
};
static_assert(sizeof(AirControl) == 0x64, "AirControl layout");

// The last few directions the squeezil slipped in, summed (0x1c bytes). Slip uses the sum to see whether the squeezil is still
// really going somewhere or only jittering. Reconstructed: 0x456da0-0x457040.
struct DirectionHistory {
    int       size;             // 0x00 slots (one more than the number of directions summed)
    Vector3f *slots;            // 0x04 heap array with a count word (Container.h NewArray)
    Vector3f  sum;              // 0x08 of the directions in every slot but the next one
    int       next;             // 0x14 the slot the next direction goes into
    int       added;            // 0x18 directions added since the last Clear

    void Init(int n);           // @00456da0 n directions
    void Destroy();             // @00456e60
    void Clear();               // @00456ef0
    Vector3f *Add(const Vector3f *direction);   // @00456f40 returns the new sum
};
static_assert(sizeof(DirectionHistory) == 0x1c, "DirectionHistory layout");

// The surface type under the squeezil (0x0c bytes, destructor 0x44f590)
struct SurfaceInfo {
    int    type;                // 0x00 the type number of the face (0: none)
    String typeName;            // 0x04 its name (" none")
    String value;               // 0x08 the face's property of that name ("none"); a change is logged and reported to the place
};

// The objects the soul is given. Neither class is reconstructed.
struct Inventory;               // 0x416010-0x4165e0 (RTTI "Inventory"): goodie lists by name, and one held goodie
struct Possessions {            // the soul's second constructor argument, destroyed with it (class not identified)
    void     **vtbl;            // 0x00 slot 4 destroys it
    Inventory *inventory;       // 0x04 (read by 0x448990)
};

enum SqueezilSoulSlot {
    SQS_DESTROY = 0, SQS_GET_INTERFACE = 1, SQS_IS_ACTIVE = 2, SQS_TOUCH = 4, SQS_UPDATE = 5, SQS_HIT = 9, SQS_GET_PHYSICS = 10,
    SQS_IS_ATTACKING = 11, SQS_SPRAY_NUTS = 12, SQS_DIE = 13, SQS_FREEZE = 15, SQS_UNFREEZE = 16, SQS_GET_BODY = 17,
    SQS_GET_POSSESSIONS = 18, SQS_TAKE_POSSESSIONS = 19, SQS_GET_DEFAULT_PHYSICS = 20, SQS_RESET_SURFACE = 21,
    SQS_UPDATE_SURFACE = 22, SQS_CAN_ENTER = 23, SQS_SET_STATE = 24, SQS_GET_STATE = 25, SQS_IS_DYING = 26,
    SQS_GROUND_STATE_FOR_SPEED = 27, SQS_TURN = 28, SQS_SPIT = 29, SQS_SET_SPIT_STRENGTH = 30, SQS_STEER = 31,
    SQS_ROTATE_ON_SURFACE = 32, SQS_WOULD_SLIP = 33, SQS_MOVE_ON_GROUND = 34, SQS_START_TUMBLE = 35, SQS_SPIT_NUT = 36,
    SQS_START_JUMP = 37, SQS_ADJUST_NEXT_STATE = 38, SQS_READ_GROUND_INPUT = 39, SQS_READ_AIR_INPUT = 40,
    SQS_GET_GROUND_MOTION = 41, SQS_GET_AIR_MOTION = 42,
    SQS_GROUND_MOVE = 43, SQS_SLIP = 44, SQS_FLY = 45, SQS_HOP_START = 46, SQS_SPIT_STANDING = 47, SQS_ATTACK = 48,
    SQS_STUMBLE = 49, SQS_LIE_DOWN = 50, SQS_GET_UP = 51, SQS_LAND = 52, SQS_DYING = 53, SQS_SKID = 54, SQS_JUMP_AIR = 55,
    SQS_HOP_AIR = 56, SQS_SKIHOP_AIR = 57, SQS_TUMBLE = 58, SQS_DYING_IN_AIR = 59
};

// SqueezilSoul : PhysicalSoul : SimulationSoul : Soul : MessageMap, and GrantMessageMap : MessageMap at +4 (RTTI). The base classes
// add nothing but their vtables, apart from the two words of GrantMessageMap. Subclasses: SqueezilPlayerSoul (0x41ee30),
// SqueezilRoboSoul (0x4043b0, its fields start at 0x80), SqueezilRegionSoul (0x436700).
struct SqueezilSoul {
    void **vtbl;                    // 0x00 @0049f74c
    void **grantVtbl;               // 0x04 @0049f72c, the GrantMessageMap part (the goodie pickup, Grant)
    int    finished;                // 0x08 set when the dying is over (GrantMessageMap's field; the reader is not here)
    int    spitPhase;               // 0x0c SpitStanding's progress (GrantMessageMap's second field, used by this class)
    DirectionHistory directions;    // 0x10 4 directions
    int    canSlip;                 // 0x2c 1; Skid clears it while it moves the squeezil
    float  pitchTarget;             // 0x30 the pitch Steer levels the flight towards (kNoInput: none)
    int    limitLanding;            // 0x34 -1; on entering SS_LAND 1 (0 after a tumble): clip the landing speed to MaxLandingSpeed
    int    stateAtDeath;            // 0x38 the state Die found (the flight model DyingInAir keeps using)
    int    state;                   // 0x3c SqueezilState
    int    previousState;           // 0x40
    int    frozen;                  // 0x44 Freeze / Unfreeze count: no spitting, no attack pushes while it is not 0
    SqueezilPhysics *physics;       // 0x48 this soul's copy
    int    unknown4c;               // 0x4c 0, not used here
    float  hopStrength;             // 0x50 -1 until the hop is let go; scales the hop's speeds
    float  spitStrength;            // 0x54 -1 until the spit is let go; scales SpitVelocity
    int    spitRequested;           // 0x58 Spit was asked for in a state that can't spit; the next SetSpitStrength is ignored
    float  leverSpin;               // 0x5c Slip's turning speed from the slope's pull (degrees per second)
    SurfaceInfo surface;            // 0x60
    Possessions *possessions;       // 0x6c
    void  *place;                   // 0x70 the PlaceSimulation the squeezil is in
    SqueezilBody *body;             // 0x74
    float  tumbleTime;              // 0x78 seconds of tumbling left
    float  jumpStartY;              // 0x7c the height a jump started at (only written)

    // @0044f1a0 (thiscall, 3 arguments) the first one made reads the [Physics] settings
    SqueezilSoul *Init(SqueezilBody *body, Possessions *possessions, void *place);
    void  Destruct();                                           // @0044f600
    SqueezilSoul *DestroyAndFree(int flags);                    // @0044f570 slot 0 (bit 0 of flags: free the memory)

    void *GetInterface(int which);                              // @004503f0 slot 1: the GrantMessageMap part for 1, else NULL
    int   IsActive();                                           // @00450410 slot 2: not dead and not frozen
    void  Touch(int otherId);                                   // @00450430 slot 4: another creature ran into this one
    void  Update();                                             // @00450e90 slot 5: one frame
    void  Hit(int knock, int sprayNuts, int unused);            // @004506d0 slot 9
    SqueezilPhysics *GetPhysics()   { return physics; }         // @00404fd0 slot 10
    int   IsAttacking();                                        // @004509d0 slot 11: sliding attack, or diving with the cannon clip
    void  SprayNuts();                                          // @00450740 slot 12: drop a share of the nuts carried
    void  Die(int how);                                         // @00450a00 slot 13
    void  Freeze()                  { frozen++; }               // @00456990 slot 15
    void  Unfreeze();                                           // @004569a0 slot 16
    SqueezilBody *GetBody()         { return body; }            // @00404fc0 slot 17
    Possessions *GetPossessions()   { return possessions; }     // @00450300 slot 18
    Possessions *TakePossessions();                             // @00450310 slot 19: hands them over (the soul forgets them)
    SqueezilPhysics *GetDefaultPhysics();                       // @00404fe0 slot 20
    void  ResetSurface();                                       // @00451500 slot 21: no surface type
    void  UpdateSurface(Contact contact);                       // @00451270 slot 22 (the contact by value)
    int   CanEnter(int newState);                               // @00404ff0 slot 23
    void  SetState(int newState, int blend);                    // @00450320 slot 24
    int   GetState()                { return state; }           // @00405010 slot 25
    int   IsDying();                                            // @00405020 slot 26: dying or dead (SS_DEAD may follow the state)
    int   GroundStateForSpeed(float speed);                     // @00456590 slot 27
    void  Turn(float radians);                                  // @004548a0 slot 28
    int   Spit();                                               // @004569c0 slot 29: the state to go to (SS_NONE: none)
    void  SetSpitStrength(float strength);                      // @00456b20 slot 30
    void  Steer(AirControl *c, float levelTo, float *matrix, float seconds);                      // @00451a50 slot 31
    void  RotateOnSurface(float *matrix, Vector3f *v, Contact *contact, float radians);           // @004517e0 slot 32
    int   WouldSlip(float push, const float *matrix, const Contact *contact);                     // @00452900 slot 33
    void  MoveOnGround(float seconds);                          // @00451fd0 slot 34
    void  StartTumble();                                        // @00452bb0 slot 35
    void  SpitNut(float speed);                                 // @004530f0 slot 36
    void  StartJump(float upSpeed, float fwdSpeed, float upDistance);   // @00452be0 slot 37
    void  AdjustNextState(int *next);                           // @00450c20 slot 38: walking <-> scampering by the slope
    // slots 39, 40: the subclass's input (pure virtual here), (control, int *next)
    void  GetGroundMotion(int forState, GroundControl *c);      // @00456b90 slot 41
    void  GetAirMotion(int forState, AirControl *c);            // @00456ca0 slot 42

    // the state handlers (slots 43-59), (control, wished-for next state)
    void  GroundMove(GroundControl *c, int next);               // @004540c0 slot 43 stand, walk, run, scamper
    void  Slip(GroundControl *c, int next);                     // @00453580 slot 44
    void  Fly(AirControl *c, int next);                         // @00454b40 slot 45 fly, dive (and every other flight)
    void  HopStart(GroundControl *c, int next);                 // @004558f0 slot 46
    void  SpitStanding(GroundControl *c, int next);             // @00455f10 slot 47
    void  Attack(GroundControl *c, int next);                   // @004560d0 slot 48
    void  Stumble(GroundControl *c, int next);                  // @00456350 slot 49
    void  LieDown(GroundControl *c, int next);                  // @00456430 slot 50
    void  GetUp(GroundControl *c, int next);                    // @004564b0 slot 51
    void  Land(GroundControl *c, int next);                     // @004565e0 slot 52
    void  Dying(GroundControl *c, int next);                    // @00456500 slot 53
    void  Skid(GroundControl *c, int next);                     // @00456800 slot 54
    void  JumpAir(AirControl *c, int next);                     // @004558a0 slot 55
    void  HopAir(AirControl *c, int next);                      // @00455a90 slot 56
    void  SkihopAir(AirControl *c, int next);                   // @00455c00 slot 57
    void  Tumble(AirControl *c, int next);                      // @00454ab0 slot 58
    void  DyingInAir(AirControl *c, int next);                  // @00456530 slot 59
    void  JumpFlight(AirControl *c, int next);                  // @00455c90 (not virtual) any jump in the air

    // GrantMessageMap part (secondary vtable 0x49f72c; `this` is soul + 4 there, the hook adjusts it)
    int   Grant(const char *goodie, int value);                 // @004504f0 slot 4 there: pick up a goodie; 0 when it can't
};
static_assert(sizeof(SqueezilSoul) == 0x80, "SqueezilSoul layout");

// The state tables (the original's, at 0x49ed40, 0x49eda0 and 0x49f5e8)
extern const int kStateOnGround[SS_COUNT + 1];
extern const int kStateMayFollow[SS_COUNT][SS_COUNT];   // [from][to]
extern const int kStateClip[SS_COUNT];                  // SqueezilMode

// Settings the scripts change (cdecl; not virtual)
void SetFloorLevelScript(const char *name);     // @00450b90 (kFloorLevelScript, Original.h)
void SetFloorLevel(float y);                    // @00450c00 (kFloorLevel)
void SetMaxLandingSpeed(float speed);           // @00450c10 in the shared defaults (souls made later copy it)

// Shared helpers of the movement code (not virtual; the original passes `this` but never uses it)
// @00451630 How much a speed changes in `seconds` on its way to `target`: speeding up by `accel`, slowing down by `decel`, both
// when it has to reverse; never past the target.
float SpeedStep(float current, float accel, float decel, float target, float seconds);
// @00451740 How far an angle (degrees) turns in `seconds` at `rate` towards `target`, the short way round; never past it.
float AngleStep(float current, float rate, float target, float seconds);
