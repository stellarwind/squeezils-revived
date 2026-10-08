// SqueezilSoul: the part of a squeezil that decides how it moves (walking, running, scampering, flying, diving, jumping, hopping,
// slipping, attacking, spitting nuts). Reconstructed so far: the physics settings it reads from the ini, SQZ.EXE 0x44f700.
// The class itself (constructor 0x44f1a0, about 60 virtual functions 0x44f570-0x456d90) is still original; see
// SQUEEZILSOUL_BRIEF.md for what is known about it.
//
// Every squeezil soul owns a copy of the settings (its +0x48); the copy is taken from the shared defaults at 0x4c7828, which the
// first soul made reads from the [Physics] section. The block starts with the settings every physical soul shares (gravity and
// friction, read by PhysicalSoul at 0x446ed0).
#pragma once
#include "Vector3f.h"

// PhysicalSoul's settings (0x24 bytes; the shared copy is at 0x4c7768, read from [Physics] Gravity / StaticFriction /
// KineticFriction by 0x446ed0).
struct PhysicalSettings {
    Vector3f gravity;           // 0x00 (0, Gravity, 0); Gravity defaults to -32
    Vector3f down;              // 0x0c (0, +-1, 0): the direction gravity pulls
    float    gravityStrength;   // 0x18 |Gravity|
    float    kineticFriction;   // 0x1c default 0.6
    float    staticFriction;    // 0x20 default 1
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
    FlightModel  third;             // 0x13c dive's, with no angle-of-attack adjustment either (use not identified)
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
