// ReadSqueezilPhysics (SQZ.EXE 0x44f700): fills the shared default physics block from [Physics] (draft from Repo/legacy_gcc/SqueezilSoul.cpp).
#include "IniReader/IniReader.h"
#include <math.h>
struct PsVec { float x, y, z; PsVec &operator=(const PsVec &o) { x = o.x; y = o.y; z = o.z; return *this; } };
struct PhysicalSettings { PsVec gravity, down; float gravityStrength, kineticFriction, staticFriction; };
struct GroundMotion { float acceleration, deceleration, speed, turnAccel, turnDecel, turnRate; };
struct FlightModel {
    float diveAngle, climbAngle, diveAdjustRate;
    float turnAccel, turnDecel, turnRate;
    float bank, pitchCorrectionRate;
    float controlCutoff, speedFactor;
    float dragZExp, dragZMult, dragYExp, dragYMult, dragXExp, dragXMult;
    float liftExp, liftMult;
    float aoaAdjustMult, stallAdjustMult;
    float tuckLiftFactor, climbLiftFactor, climbPitchFactor;
};
struct JumpModel {
    float upSpeed, fwdSpeed, upDistance, maxLinearSpeed, linearAccel;
    float dragXMult, dragYMult, dragZMult, aoaAdjustMult, gravityFactor;
};
struct LeverMotion {
    float speedCutoff, distanceCutoff, turnAccel, turnDecel, turnRate;
    float minSpeed, pushIncrement, frictionScale, controlSpeed, leverArm, leverWeightFactor;
};
struct SqueezilPhysics {
    PhysicalSettings physical;
    GroundMotion walk, run, scamper, skid;
    FlightModel fly, dive, tumble;
    JumpModel jump, hop, skihop;
    LeverMotion attack, slip;
    float clawGrip, crouchGrip, maxLandingSpeed, tumbleDuration, spitVelocity;
    float walkToScamperAngle, scamperToWalkAngle, cornerAngle;
    int unknown288;
    float fwdTapFactor, rotTapFactor, maxWalkSpeed, minStumbleSpeed, maxRunSpeed;
};
extern SqueezilPhysics *gDefaultPhysics;                           // 0x4c7ad0
extern PhysicalSettings gPhysicalSettings;                         // 0x4c7768
extern float gLevelNutSprayPercent, gSpitNutSprayPercent;          // 0x4c7820, 0x4c7b0c
#define Size(ini, key, def) ((float)fabs((ini).GetFloat((key), (def))))
void ReadSqueezilPhysics()
{
    IniReader ini;
    ini.SetSection("Physics");
    gDefaultPhysics->physical = gPhysicalSettings;
    SqueezilPhysics *p = gDefaultPhysics;

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

    p->fly.diveAngle           = Size(ini, "DiveAngle", 42.0f);
    p->fly.climbAngle          = Size(ini, "ClimbAngle", -22.0f);
    p->fly.diveAdjustRate      = Size(ini, "DiveAdjustRate", 30.0f);
    p->fly.turnAccel           = Size(ini, "FlyTurnAccel", 10.0f);
    p->fly.turnDecel           = Size(ini, "FlyTurnDecel", -20.0f);
    p->fly.turnRate            = Size(ini, "FlyTurnRate", 30.0f);
    p->fly.bank                = Size(ini, "FlyBank", 60.0f);
    p->fly.pitchCorrectionRate = Size(ini, "PitchCorrectionRate", 45.0f);
    p->fly.controlCutoff       = ini.GetFloat("FlyControlCutoff", 10.0f);
    p->fly.speedFactor         = ini.GetFloat("FlySpeedFactor", 1.0f);
    p->fly.dragZExp            = ini.GetFloat("DragZExp", 1.0f);
    p->fly.dragZMult           = ini.GetFloat("DragZMult", 0.01f);
    p->fly.dragYExp            = ini.GetFloat("DragYExp", 1.0f);
    p->fly.dragYMult           = ini.GetFloat("DragYMult", 0.1f);
    p->fly.dragXExp            = ini.GetFloat("DragXExp", 1.0f);
    p->fly.dragXMult           = ini.GetFloat("DragXMult", 1.0f);
    p->fly.liftExp             = ini.GetFloat("LiftExp", 1.0f);
    p->fly.liftMult            = ini.GetFloat("LiftMult", 0.45f);
    p->fly.aoaAdjustMult       = ini.GetFloat("AOAAdjustMult", 5.0f);
    p->fly.stallAdjustMult     = ini.GetFloat("StallAdjustMult", 8.0f);
    p->fly.tuckLiftFactor      = ini.GetFloat("TuckLiftFactor", 0.2f);
    p->fly.climbLiftFactor     = ini.GetFloat("ClimbLiftFactor", 1.6f);
    p->fly.climbPitchFactor    = ini.GetFloat("ClimbPitchFactor", 1.35f);

    p->dive = p->fly;
    p->dive.dragZExp      = ini.GetFloat("DiveDragZExp", 1.0f);
    p->dive.dragZMult     = ini.GetFloat("DiveDragZMult", 0.6f);
    p->dive.dragYExp      = ini.GetFloat("DiveDragYExp", 1.0f);
    p->dive.dragYMult     = ini.GetFloat("DiveDragYMult", 0.6f);
    p->dive.dragXExp      = ini.GetFloat("DiveDragXExp", 1.0f);
    p->dive.dragXMult     = ini.GetFloat("DiveDragXMult", 1.0f);
    p->dive.liftExp       = ini.GetFloat("DiveLiftExp", 1.0f);
    p->dive.liftMult      = ini.GetFloat("DiveLiftMult", 0.0f);
    p->dive.aoaAdjustMult = ini.GetFloat("DiveAOAAdjustMult", 8.0f);
    p->dive.stallAdjustMult = 0.0f;

    p->tumble = p->dive;
    p->tumbleDuration = ini.GetFloat("TumbleDuration", 1.5f);
    p->tumble.aoaAdjustMult = 0.0f;

    p->jump.upSpeed        = ini.GetFloat("JumpUpSpeed", 3.0f);
    p->jump.fwdSpeed       = ini.GetFloat("JumpFwdSpeed", 6.0f);
    p->jump.upDistance     = ini.GetFloat("JumpUpDistance", 0.1f);
    p->jump.maxLinearSpeed = ini.GetFloat("JumpMaxLinearSpeed", 5.0f);
    p->jump.linearAccel    = ini.GetFloat("JumpLinearAccel", 2.0f);
    p->jump.dragXMult      = ini.GetFloat("JumpDragXMult", p->dive.dragXMult);
    p->jump.dragYMult      = ini.GetFloat("JumpDragYMult", p->dive.dragYMult);
    p->jump.dragZMult      = ini.GetFloat("JumpDragZMult", p->dive.dragZMult);
    p->jump.aoaAdjustMult  = ini.GetFloat("JumpAOAAdjustMult", 0.0f);
    p->jump.gravityFactor  = ini.GetFloat("JumpGravityFactor", 1.0f);

    p->hop.upSpeed        = ini.GetFloat("HopUpSpeed", 6.0f);
    p->hop.fwdSpeed       = ini.GetFloat("HopFwdSpeed", 4.0f);
    p->hop.upDistance     = ini.GetFloat("HopUpDistance", 0.1f);
    p->hop.maxLinearSpeed = ini.GetFloat("HopMaxLinearSpeed", p->jump.maxLinearSpeed);
    p->hop.linearAccel    = ini.GetFloat("HopLinearAccel", 2.0f);
    p->hop.dragXMult      = ini.GetFloat("HopDragXMult", p->dive.dragXMult);
    p->hop.dragYMult      = ini.GetFloat("HopDragYMult", p->dive.dragYMult);
    p->hop.dragZMult      = ini.GetFloat("HopDragZMult", p->dive.dragZMult);
    p->hop.aoaAdjustMult  = ini.GetFloat("HopAOAAdjustMult", 0.0f);
    p->hop.gravityFactor  = ini.GetFloat("HopGravityFactor", 1.0f);

    p->skihop.upSpeed        = ini.GetFloat("SkihopUpSpeed", p->jump.upSpeed);
    p->skihop.fwdSpeed       = ini.GetFloat("SkihopFwdSpeed", p->jump.fwdSpeed);
    p->skihop.upDistance     = ini.GetFloat("SkihopUpDistance", p->jump.upDistance);
    p->skihop.maxLinearSpeed = ini.GetFloat("SkihopMaxLinearSpeed", p->jump.maxLinearSpeed);
    p->skihop.linearAccel    = ini.GetFloat("SkihopLinearAccel", p->jump.linearAccel);
    p->skihop.dragXMult      = ini.GetFloat("SkihopDragXMult", p->dive.dragXMult);
    p->skihop.dragYMult      = ini.GetFloat("SkihopDragYMult", p->dive.dragYMult);
    p->skihop.dragZMult      = ini.GetFloat("SkihopDragZMult", p->dive.dragZMult);
    p->skihop.aoaAdjustMult  = ini.GetFloat("SkihopAOAAdjustMult", 0.0f);
    p->skihop.gravityFactor  = ini.GetFloat("SkihopGravityFactor", 1.0f);

    p->slip.speedCutoff       = Size(ini, "SlipSpeedCutoff", p->walk.speed * 0.25f);
    p->slip.distanceCutoff    = Size(ini, "SlipDistanceCutoff", 0.5f);
    p->slip.turnAccel         = Size(ini, "SlipTurnAccel", 0.0f);
    p->slip.turnDecel         = Size(ini, "SlipTurnDecel", 0.0f);
    p->slip.turnRate          = Size(ini, "SlipTurnRate", 0.0f);
    p->slip.minSpeed          = 1.0f;
    p->slip.pushIncrement     = 0.0f;
    p->slip.frictionScale     = 1.0f;
    p->slip.controlSpeed      = ini.GetFloat("SlipControlSpeed", 0.0f);
    p->slip.leverArm          = ini.GetFloat("SlipLeverArm", 1.0f);
    p->slip.leverWeightFactor = ini.GetFloat("SlipLeverWeightFactor", 0.5f);

    p->attack = p->slip;
    p->attack.turnAccel         = Size(ini, "AttackTurnAccel", 90.0f);
    p->attack.turnDecel         = Size(ini, "AttackTurnDecel", 75.0f);
    p->attack.turnRate          = Size(ini, "AttackTurnRate", 100.0f);
    p->attack.minSpeed          = Size(ini, "AttackMinSpeed", p->walk.speed * 0.25f);
    p->attack.pushIncrement     = Size(ini, "AttackPushIncrement", 1.0f);
    p->attack.frictionScale     = ini.GetFloat("AttackFrictionScale", 0.25f);
    p->attack.controlSpeed      = Size(ini, "AttackControlSpeed", p->run.speed);
    p->attack.leverArm          = ini.GetFloat("AttackLeverArm", 1.0f);
    p->attack.leverWeightFactor = ini.GetFloat("AttackLeverWeightFactor", 0.5f);

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
