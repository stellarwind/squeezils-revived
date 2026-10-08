// SqueezilSoul: see SqueezilSoul.h. Reconstructed so far: SQZ.EXE 0x44f700-0x4502f4.
#include <math.h>
#include "SqueezilSoul.h"
#include "IniReader.h"

// The shared defaults and the settings outside the block. They stay at their original addresses while the soul is original code.
static SqueezilPhysics *&gDefaultPhysics     = *(SqueezilPhysics **)0x4c7ad0;   // points at the block at 0x4c7828
static PhysicalSettings &gPhysicalSettings  = *(PhysicalSettings *)0x4c7768;    // PhysicalSoul's
static float &gLevelNutSprayPercent          = *(float *)0x4c7820;
static float &gSpitNutSprayPercent           = *(float *)0x4c7b0c;

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

    p->third = dive;
    p->tumbleDuration = ini.GetFloat("TumbleDuration", 1.5f);
    p->third.aoaAdjustMult = 0.0f;

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
