// SqueezilSoul's movement (see SqueezilSoul.h): moving over surfaces, the flight model and the state handlers. Reconstructed from
// SQZ.EXE 0x450c20-0x450e40, 0x451630-0x4569c0.
//
// Units: speeds in units per second, angles in degrees (the body's angular speeds in degrees per second), the frame time in
// seconds. On the ground the body's matrix rows are right, up, forward and position, and its Contact says which face of which
// surface copy it stands on; MoveAlongSurface (original code of the place) carries it along the faces.
#include <math.h>
#include <string.h>
#include "SqueezilSoulInternal.h"
#include "Simulation.h"
#include "Script.h"
#include "FrameProp.h"

// ---- small helpers ----

// @00451630 The original tests signs by the sign bit and zero by the other bits; the float comparisons below give the same
// answers (also for -0).
float SpeedStep(float current, float accel, float decel, float target, float seconds)
{
    float currentSign = current < 0.0f ? -1.0f : 1.0f;
    float targetSign = target < 0.0f ? -1.0f : 1.0f;
    float currentSize = current * currentSign;
    float targetSize = target * targetSign;
    float rate;
    if (current != 0.0f && target != 0.0f && currentSign != targetSign)
        rate = (decel + accel) * targetSign;            // reversing: brake and speed up the other way at once
    else if (targetSize < currentSize)
        rate = -(decel * currentSign);
    else if (targetSize > currentSize)
        rate = accel * targetSign;
    else
        rate = 0.0f;
    float step = rate * seconds;
    float difference = target - current;
    return fabsf(step) <= fabsf(difference) ? step : difference;
}

// @00451740
float AngleStep(float current, float rate, float target, float seconds)
{
    float difference = target - current;
    if (difference > 180.0f)
        difference -= 360.0f;
    if (difference < -180.0f)
        difference += 360.0f;
    float size = fabsf(difference);
    float step = (difference < 0.0f ? -rate : rate) * seconds;
    return fabsf(step) <= size ? step : difference;
}

// The level's floor height: a squeezil going below it is held at it, and the floor's script runs (once, as it crosses)
static void CheckFloorLevel(SqueezilSoul *soul, Vector3f *position, float oldY)
{
    if (position->y < *kFloorLevel && oldY > *kFloorLevel) {
        position->y = *kFloorLevel;
        if (strlen(kFloorLevelScript->c_str()) != 0) {
            LocationSimulation *sim = (LocationSimulation *)GetCurrentSimulation();
            ScriptCaller caller;
            ScriptCallerInitSoul(&caller, soul, sim->FindIdBySoul(soul), NULL);
            PostFunction(kFloorLevelScript->c_str(), &caller, 0, 0);
        }
    }
}

// The script function a surface type names, run by the squeezil that touched it
static void RunScript(SqueezilSoul *soul, const String &name)
{
    if (strlen(name.c_str()) != 0) {
        LocationSimulation *sim = (LocationSimulation *)GetCurrentSimulation();
        ScriptCaller caller;
        ScriptCallerInitSoul(&caller, soul, sim->FindIdBySoul(soul), NULL);
        PostFunction(name.c_str(), &caller, 0, 0);
    }
}

// The angle (degrees) between the body's up and the world's up
static float SlopeAngle(SqueezilSoul *soul)
{
    Vector3f worldUp = soul->physics->physical.down * -1.0f;
    Vector3f up = Row(BodyCall::Matrix(SoulCall::Body(soul)), 1);
    return (float)acos(Dot(up, worldUp)) * kDegreesPerRadian;
}

// ---- turning ----

// @00451a50 The flight's turn: the yaw speed follows the turn wish, the pitch follows the pitch wish (weaker when slow, and by
// ClimbPitchFactor when climbing) plus the pull towards pitchTarget, and the roll banks with the turn. The matrix is rebuilt from
// the three angles at its old position; the body keeps the angles and the pitch and roll speeds.
void SqueezilSoul::Steer(AirControl *c, float levelTo, float *m, float seconds)
{
    const FlightModel &f = c->flight;
    float pitch, yaw, roll;
    BodyCall::GetOrientation(SoulCall::Body(this), &pitch, &yaw, &roll);
    float pitchRate, yawRate, rollRate;
    BodyCall::GetAngles(SoulCall::Body(this), &pitchRate, &yawRate, &rollRate);

    yawRate += SpeedStep(yawRate, f.turnAccel, f.turnDecel, c->turn * f.turnRate, seconds);
    yaw += yawRate * seconds;
    if (yaw >= 360.0f)
        yaw -= 360.0f;
    if (yaw < 0.0f)
        yaw += 360.0f;

    float pitchStep;
    if (c->pitch != kNoInput) {
        float target = (c->pitch < 0.0f ? f.climbAngle : f.diveAngle) * c->pitch;
        pitchStep = AngleStep(pitch, f.diveAdjustRate, target, seconds);
        SqueezilBody *b = SoulCall::Body(this);
        float speed = BodyCall::Speed(b);
        float *bm = BodyCall::Matrix(SoulCall::Body(this));
        Vector3f direction = BodyCall::Direction(SoulCall::Body(this));
        float forwardSpeed = (direction.y * bm[9] + direction.z * bm[10] + bm[8] * direction.x) * speed;
        if (forwardSpeed < f.controlCutoff) {            // too slow to steer fully: fades out over the last unit of speed
            float control = f.controlCutoff - 1.0f <= forwardSpeed ? forwardSpeed - f.controlCutoff + 1.0f : 0.0f;
            pitchStep *= control;
        }
        if (c->pitch < 0.0f)
            pitchStep *= f.climbPitchFactor;
    } else {
        pitchStep = 0.0f;
    }
    float levelStep = levelTo != kNoInput ? AngleStep(pitch, f.pitchCorrectionRate, levelTo, seconds) : 0.0f;
    float pitchChange = levelStep + pitchStep;
    pitch += pitchChange;

    float rollChange = AngleStep(roll, f.turnRate, -(yawRate / f.turnRate) * f.bank, seconds);
    roll += rollChange;
    if (roll > 180.0f)                  // (half a turn, not a whole one; kept as the original has it)
        roll -= 180.0f;
    else if (roll < -180.0f)
        roll += 180.0f;

    Vector3f position = Row(m, 3);
    static const float kIdentity[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
    memcpy(m, kIdentity, sizeof kIdentity);
    float turn[16];
    RotationY(turn, yaw * kRadiansPerDegree);
    TurnMatrix(m, turn);
    RotationX(turn, pitch * kRadiansPerDegree);
    TurnMatrix(m, turn);
    RotationZ(turn, roll * kRadiansPerDegree);
    TurnMatrix(m, turn);
    SetTranslation(m, position);

    if (seconds > 0.001f) {
        pitchRate = pitchChange / seconds;
        rollRate = rollChange / seconds;
    } else {
        pitchRate = rollRate = 0.0f;
    }
    BodyCall::SetAngles(SoulCall::Body(this), pitchRate, yawRate, rollRate);
    BodyCall::SetOrientation(SoulCall::Body(this), pitch, yaw, roll);
}

// @004517e0 Turns the matrix about its own up axis (the position stays), keeps the contact's matrix relative to the face in step,
// and turns v the same way.
void SqueezilSoul::RotateOnSurface(float *m, Vector3f *v, Contact *contact, float radians)
{
    Vector3f position = Row(m, 3);
    Vector3f up = Row(m, 1);
    float turn[16], turned[16];
    RotationAbout(turn, up, radians);
    MultiplyMatrix(turned, m, turn);
    memcpy(m, turned, sizeof turned);
    SetTranslation(m, position);
    Vector3f onFace = Row(contact->matrix, 3);
    float relative[16];
    memcpy(contact->matrix, FaceRelativeMatrix(NavSurfaceFaceOf(GetPlaceSurfaces(place), contact), relative, m), sizeof relative);
    SetTranslation(contact->matrix, onFace);
    *v = TransformPoint(*v, turn);
}

// @004548a0
void SqueezilSoul::Turn(float radians)
{
    SqueezilBody *b = SoulCall::Body(this);
    float m[16];
    memcpy(m, BodyCall::Matrix(b), sizeof m);
    Contact contact = *BodyCall::GetContact(SoulCall::Body(this));
    Vector3f direction = BodyCall::Direction(SoulCall::Body(this));
    float speed = BodyCall::Speed(SoulCall::Body(this));
    SoulCall::RotateOnSurface(this, m, &direction, &contact, radians);
    BodyCall::SetMatrix(SoulCall::Body(this), m);
    BodyCall::SetVelocity(SoulCall::Body(this), direction, speed);
    BodyCall::SetContact(SoulCall::Body(this), &contact);
}

// ---- the ground ----

// @00452900 Whether a push along the surface (plus the slope's own pull) beats the grip: static friction on the weight pressing
// into the surface, plus ClawGrip, plus CrouchGrip when scampering. On a wall or ceiling nothing presses in and the whole weight
// pulls; running there always slips. A slippery surface type always slips; with canSlip cleared nothing does.
int SqueezilSoul::WouldSlip(float push, const float *m, const Contact *contact)
{
    float staticFriction = physics->physical.staticFriction;
    float clawGrip = physics->clawGrip;
    float crouchGrip = physics->crouchGrip;
    int current = SoulCall::GetState(this);
    float mass = BodyCall::Mass(SoulCall::Body(this));
    Vector3f gravity = physics->physical.gravity;
    Vector3f up = Row(m, 1);
    if (NavSurfaceIsSlippery(GetPlaceSurfaces(place), contact))
        return 1;
    if (!canSlip)
        return 0;
    float into = up.z * gravity.z + up.y * gravity.y + up.x * gravity.x;
    float sign = into < 0.0f ? -1.0f : 1.0f;
    int wall = sign == 1.0f || fabs(into) < (double)0.0001f;
    if (current == SS_RUN) {
        if (wall)
            return 1;
    }
    float pressing, pulling;
    if (current == SS_RUN || !wall) {
        pressing = -(into * mass);
        Vector3f side = Cross(gravity, up);
        Normalize(side);
        Vector3f downhill = Cross(side, up);
        pulling = -(Dot(downhill, gravity) * mass);
    } else {
        pulling = Length(gravity) * mass;
        pressing = 0.0f;
    }
    float grip = staticFriction * pressing + clawGrip;
    if (current == SS_SCAMPER || current == SS_SCAMPER_STOP)
        grip += crouchGrip;
    return fabsf(push) * mass + pulling > grip;
}

// @00451fd0 Carries the body `seconds` along the surface at its speed. Over a sharp corner (more than CornerAngle) where it would
// slip, it either goes off the edge (moving into the new face) or bounces back (from flat ground onto a steep face). Off the
// surface it hops, jumps or tumbles into the air.
void SqueezilSoul::MoveOnGround(float seconds)
{
    SqueezilBody *b = SoulCall::Body(this);
    Contact contact = *BodyCall::GetContact(b);
    Vector3f direction = BodyCall::Direction(SoulCall::Body(this));
    float speed = BodyCall::Speed(SoulCall::Body(this));
    float m[16];
    memcpy(m, BodyCall::Matrix(SoulCall::Body(this)), sizeof m);
    Float3 position = ToFloat3(Row(m, 3));
    Vector3f oldPosition = ToVector(position);
    Float3 right = ToFloat3(Row(m, 0));
    Float3 up = ToFloat3(Row(m, 1));
    Float3 forward = ToFloat3(Row(m, 2));
    Float3 dir = ToFloat3(direction);
    // (the original works out the direction's forward part here and never uses it)
    int moved = MoveAlongSurface(GetPlaceSurfaces(place), this, &position, &right, &up, &forward, &dir, speed * seconds,
                                 &contact, &position, &right, &up, &forward, &dir, &contact);
    int bounce = 0;
    if (moved && NavSurfaceBlocksWalking(GetPlaceSurfaces(place), &contact))
        bounce = 1;
    if (moved) {
        Contact *now = BodyCall::GetContact(SoulCall::Body(this));
        if (now->face != contact.face || BodyCall::GetContact(SoulCall::Body(this))->second != contact.second) {
            Vector3f oldUp = Row(m, 1);
            float corner = (float)acos(oldUp.y * up.y + oldUp.z * up.z + oldUp.x * up.x) * kDegreesPerRadian;
            if (corner > physics->cornerAngle && SoulCall::WouldSlip(this, 0.0f, m, &contact)) {
                Vector3f going = BodyCall::Direction(SoulCall::Body(this));
                if (going.z * up.z + going.y * up.y + going.x * up.x > 0.0f) {
                    moved = 0;                                  // over the edge
                } else {
                    Vector3f worldUp = physics->physical.down * -1.0f;
                    int steep = 0;
                    if (Dot(worldUp, oldUp) > (double)0.8)
                        steep = Dot(worldUp, ToVector(up)) < (double)0.3;
                    if (steep)
                        bounce = 1;
                }
            }
        }
    }
    if (bounce) {
        speed = -speed;
        BodyCall::SetVelocity(SoulCall::Body(this), ToVector(dir), speed);
        return;
    }
    if (!moved) {
        SetTranslation(m, oldPosition + ToVector(up) * 0.2f);
        BodyCall::SetMatrix(SoulCall::Body(this), m);
        if (SoulCall::CanEnter(this, SS_SKIHOP))
            SoulCall::SetState(this, SS_SKIHOP, 0);
        else if (SoulCall::CanEnter(this, SS_HOP))
            SoulCall::SetState(this, SS_HOP, 0);
        else if (SoulCall::CanEnter(this, SS_JUMP))
            SoulCall::SetState(this, SS_JUMP, 0);
        else
            SoulCall::StartTumble(this);
        return;
    }
    Vector3f newPosition = ToVector(position);
    CheckFloorLevel(this, &newPosition, oldPosition.y);
    SetRows(m, ToVector(right), ToVector(up), ToVector(forward), newPosition);
    Vector3f newDirection = ToVector(dir);
    Normalize(newDirection);            // (only the vector is used; the length is thrown away)
    Contact previous = *BodyCall::GetContact(SoulCall::Body(this));
    if (previous.face != contact.face)
        SoulCall::ResetSurface(this);
    BodyCall::SetMatrix(SoulCall::Body(this), m);
    BodyCall::SetVelocity(SoulCall::Body(this), newDirection, speed);
    BodyCall::SetContact(SoulCall::Body(this), &contact);
    SoulCall::UpdateSurface(this, contact);
}

// @00450c20 On a slope steeper than WalkToScamperAngle a walk becomes a scamper; below ScamperToWalkAngle a scamper becomes a walk
// again; stopping on a steep slope uses the scamper stop.
void SqueezilSoul::AdjustNextState(int *next)
{
    int current = SoulCall::GetState(this);
    if (*next == SS_WALK || (*next == SS_NONE && (current == SS_WALK || current == SS_SCAMPER))) {
        float slope = SlopeAngle(this);
        if (current == SS_WALK || current == SS_STAND) {
            if (physics->walkToScamperAngle < slope)
                *next = SS_SCAMPER;
        } else if (current == SS_SCAMPER) {
            *next = SS_NONE;
            if (physics->scamperToWalkAngle > slope)
                *next = SS_WALK;
        } else if (current == SS_SCAMPER_STOP) {
            *next = physics->scamperToWalkAngle > slope ? SS_WALK : SS_SCAMPER;
        }
    } else if (*next == SS_STAND && (current == SS_SCAMPER || current == SS_SCAMPER_STOP)) {
        if (SlopeAngle(this) > physics->scamperToWalkAngle)
            *next = SS_SCAMPER_STOP;
    }
}

// @004540c0 Standing, walking, running, scampering: turn by the turn wish, speed up or slow down along the facing by the forward
// wish, slip if the push is more than the grip, then move along the surface. Running backwards plays the walk clip.
void SqueezilSoul::GroundMove(GroundControl *c, int next)
{
    float m[16];
    memcpy(m, BodyCall::Matrix(SoulCall::Body(this)), sizeof m);
    Contact contact = *BodyCall::GetContact(SoulCall::Body(this));
    Vector3f direction = BodyCall::Direction(SoulCall::Body(this));
    Vector3f position = Row(m, 3);
    float speed = BodyCall::Speed(SoulCall::Body(this));
    float seconds = FrameSeconds();
    int startState = SoulCall::GetState(this);
    int backwards = speed < 0.0f;
    if (Dot(direction, Row(m, 2)) < 0.0f)
        backwards = !backwards;
    if (next != SS_NONE && SoulCall::CanEnter(this, next) && (next != SS_STAND || speed <= 0.01f))
        SoulCall::SetState(this, next, 0);

    float angleX, yawRate, angleZ;
    BodyCall::GetAngles(SoulCall::Body(this), &angleX, &yawRate, &angleZ);
    yawRate += SpeedStep(yawRate, c->ground.turnAccel, c->ground.turnDecel, c->turn * c->ground.turnRate, seconds);
    if (fabsf(yawRate) > 0.1f)
        SoulCall::RotateOnSurface(this, m, &direction, &contact, seconds * yawRate * kRadiansPerDegree);
    Vector3f right = Row(m, 0), up = Row(m, 1), forward = Row(m, 2);
    if (speed <= 0.01f)
        direction = forward * (c->forward < 0.0f ? -1.0f : 1.0f);
    else
        direction = forward * (backwards ? -1.0f : 1.0f);

    float change = SpeedStep(Dot(direction, forward) * speed, c->ground.acceleration, c->ground.deceleration,
                             c->forward * c->ground.speed, seconds);
    if (SoulCall::WouldSlip(this, change / seconds, m, &contact) && SoulCall::CanEnter(this, SS_SLIP)) {
        SoulCall::SetState(this, SS_SLIP, 0);
        return;
    }
    if (fabsf(change) > 0.01f || speed > 0.01f) {
        Vector3f velocity = direction * speed + forward * change;
        speed = Normalize(velocity);
        direction = velocity;
    }
    SetRows(m, right, up, forward, position);
    BodyCall::SetMatrix(SoulCall::Body(this), m);
    BodyCall::SetVelocity(SoulCall::Body(this), direction, speed);
    BodyCall::SetAngles(SoulCall::Body(this), angleX, yawRate, angleZ);
    BodyCall::SetContact(SoulCall::Body(this), &contact);
    if (SoulCall::GetState(this) == SS_RUN) {
        int nowBackwards = Dot(direction, forward) < 0.0f;
        if (speed < 0.0f)
            nowBackwards = !nowBackwards;
        if (backwards != nowBackwards || startState != SS_RUN)
            BodyCall::SetMode(SoulCall::Body(this), nowBackwards ? MODE_WALK : MODE_RUN);
    }
    float clipSpeed;
    if (SoulCall::GetState(this) == SS_RUN && BodyCall::GetMode(SoulCall::Body(this)) == MODE_WALK)
        clipSpeed = physics->walk.speed;
    else
        clipSpeed = c->ground.speed;
    BodyCall::SetClipSpeed(SoulCall::Body(this), clipSpeed);
    SoulCall::MoveOnGround(this, seconds);
}

// @00453580 Sliding on a slope: the slope's pull turns the squeezil like a lever (leverSpin), friction (FrictionScale times
// KineticFriction) and gravity change the velocity, and the slide stops when it is slow and has not really got anywhere over the
// last few frames. Lifted off the surface (it faces down) it hops off, or tumbles.
void SqueezilSoul::Slip(GroundControl *c, int next)
{
    const LeverMotion &l = c->lever;
    SqueezilPhysics *p = physics;
    float seconds = FrameSeconds();
    SoulCall::GetState(this);           // (result not used)
    float m[16];
    memcpy(m, BodyCall::Matrix(SoulCall::Body(this)), sizeof m);
    Contact contact = *BodyCall::GetContact(SoulCall::Body(this));
    Vector3f gravity = p->physical.gravity;
    float speedCutoff = l.speedCutoff, distanceCutoff = l.distanceCutoff;
    float mass = BodyCall::Mass(SoulCall::Body(this));
    Vector3f direction = BodyCall::Direction(SoulCall::Body(this));
    float speed = BodyCall::Speed(SoulCall::Body(this));
    float angleX, yawRate, angleZ;
    BodyCall::GetAngles(SoulCall::Body(this), &angleX, &yawRate, &angleZ);
    Vector3f up = Row(m, 1);
    float friction = l.frictionScale * p->physical.kineticFriction;
    float into = gravity.y * up.y + gravity.z * up.z + gravity.x * up.x;
    float sign = into < 0.0f ? -1.0f : 1.0f;

    if (sign == 1.0f || !(into <= -0.02)) {
        // not standing on anything: lift off a little and leave the surface
        SetTranslation(m, Row(m, 3) + Row(m, 1) * 0.2f);
        BodyCall::SetMatrix(SoulCall::Body(this), m);
        if (SoulCall::CanEnter(this, SS_HOP)) {
            SoulCall::SetState(this, SS_HOP, 0);
            SoulCall::ResetSurface(this);
        } else {
            SoulCall::StartTumble(this);
        }
        return;
    }

    if (l.controlSpeed != 0.0f) {
        float turnTarget = c->turn * l.turnRate;
        if (fabsf(turnTarget) <= 0.1f) {
            // no turn wish: the slope's pull turns the squeezil about its feet
            Vector3f side = Cross(gravity, up);
            Normalize(side);
            Vector3f downhill = Cross(side, up);
            float pull = downhill.z * gravity.z + downhill.y * gravity.y + downhill.x * gravity.x;
            float inertia = mass * l.leverWeightFactor * p->physical.gravityStrength * (l.leverArm * l.leverArm);
            Vector3f right = Row(m, 0);
            float sideways = downhill.y * right.y + downhill.z * right.z + downhill.x * right.x;
            leverSpin += sideways * pull / inertia * seconds;
            Vector3f forward = Row(m, 2);
            if (downhill.y * forward.y + downhill.z * forward.z + downhill.x * forward.x < 0.0f) {
                float k = fabsf(sideways);
                if (k < 0.1f)
                    k = 0.1f;
                leverSpin *= k;
            }
        }
        float step = SpeedStep(yawRate, l.turnAccel, l.turnDecel, turnTarget, seconds);
        float control = speed / l.controlSpeed;
        if (control > 1.0f)
            control = 1.0f;
        yawRate += control * step;
        float turn[16];
        RotationAbout(turn, up, yawRate * seconds * kRadiansPerDegree);
        direction = TransformPoint(direction, turn);
    }
    float spin = leverSpin + yawRate;
    if (fabsf(spin) > 0.1f) {
        Vector3f axis(0.0f, 1.0f, 0.0f);    // (the original turns this vector and throws it away)
        SoulCall::RotateOnSurface(this, m, &axis, &contact, spin * seconds * kRadiansPerDegree);
    }
    Vector3f position = Row(m, 3), right = Row(m, 0), forward = Row(m, 2);
    up = Row(m, 1);

    // the new velocity: the old one plus gravity, the surface pushing back, and friction against the movement
    float pressing = fabsf(mass * into);
    Vector3f support = up * pressing;
    Vector3f rubbing = direction * -(friction * pressing);
    Vector3f weight = gravity * mass;
    Vector3f momentum = direction * speed;
    Vector3f before = momentum + weight + support;
    Vector3f after = before + rubbing;
    float newSpeed;
    if (Dot(after, before) < 0.0f) {
        newSpeed = 0.0f;                    // friction turned it round: it stops
    } else {
        newSpeed = Normalize(after);
        Vector3f recent = *directions.Add(&after);
        if (directions.added >= directions.size) {
            float distance = Length(recent);
            if (speedCutoff > newSpeed && distanceCutoff > distance)
                newSpeed = 0.0f;
        }
    }
    SetRows(m, right, up, forward, position);
    BodyCall::SetMatrix(SoulCall::Body(this), m);
    BodyCall::SetContact(SoulCall::Body(this), &contact);
    BodyCall::SetAngles(SoulCall::Body(this), angleX, yawRate, angleZ);
    if (newSpeed > 0.01f) {
        BodyCall::SetVelocity(SoulCall::Body(this), after, newSpeed);
        SoulCall::MoveOnGround(this, seconds);
    } else {
        BodyCall::SetVelocity(SoulCall::Body(this), direction, 0.0f);
        SoulCall::SetState(this, SoulCall::CanEnter(this, next) ? next : SS_STAND, 1);
        directions.Clear();
        leverSpin = 0.0f;
    }
}

// ---- jumping ----

// @00452be0 Launches the squeezil off the ground: up and forward speeds are added to its velocity, and it is lifted upDistance
// so it starts clear of the surface. Writes a trace to the debug output.
void SqueezilSoul::StartJump(float upSpeed, float fwdSpeed, float upDistance)
{
    if (!BodyCall::IsOnGround(SoulCall::Body(this)))
        return;
    float m[16];
    memcpy(m, BodyCall::Matrix(SoulCall::Body(this)), sizeof m);
    Vector3f up = Row(m, 1), forward = Row(m, 2);
    float speed = BodyCall::Speed(SoulCall::Body(this));
    Vector3f velocity = BodyCall::Direction(SoulCall::Body(this));
    velocity = velocity * speed + up * upSpeed + forward * fwdSpeed;
    float newSpeed = Normalize(velocity);
    Vector3f position = Row(m, 3);
    DebugLog("Starting jump, upSpeed %f, fwdSpeed %f, upDist %f.\n", (double)upSpeed, (double)fwdSpeed, (double)upDistance);
    int b = ClockValueB();
    DebugLog("%d\t%d\t%f\t%f\n", ClockValueA(), b, (double)position.x, (double)position.y);
    jumpStartY = position.y;
    position = position + up * upDistance;
    BodyCall::SetVelocity(SoulCall::Body(this), velocity, newSpeed);
    SetTranslation(m, position);
    BodyCall::SetMatrix(SoulCall::Body(this), m);
    PlayEffect("jumpEffect", BodyPosition(this));
    SoulCall::ResetSurface(this);
    b = ClockValueB();
    DebugLog("%d\t%d\t%f\t%f\n", ClockValueA(), b, (double)position.x, (double)position.y);
}

// @00455c90 In the air after a jump, hop or ski-hop: the forward wish changes the speed along the facing (only while it pushes
// the way the wish points), then the flight is the tumble model with the jump's drag, under gravity times GravityFactor.
void SqueezilSoul::JumpFlight(AirControl *c, int next)
{
    if (c->pitch != kNoInput) {
        Vector3f forward = Row(BodyCall::Matrix(SoulCall::Body(this)), 2);
        Vector3f direction = BodyCall::Direction(SoulCall::Body(this));
        float speed = BodyCall::Speed(SoulCall::Body(this));
        float seconds = FrameSeconds();
        float target = c->jump.maxLinearSpeed * c->pitch;
        float change = SpeedStep((direction.z * forward.z + direction.y * forward.y + direction.x * forward.x) * speed,
                                 c->jump.linearAccel, c->jump.linearAccel, target, seconds);
        float changeSign = change < 0.0f ? -1.0f : 1.0f;
        float targetSign = target < 0.0f ? -1.0f : 1.0f;
        if (changeSign == targetSign) {
            Vector3f velocity = direction * speed + forward * change;
            float newSpeed = Normalize(velocity);
            BodyCall::SetVelocity(SoulCall::Body(this), velocity, newSpeed);
        }
        c->pitch = kNoInput;
    }
    float gravity = physics->physical.gravityStrength * physics->physical.down.y;
    physics->physical.SetGravity(gravity * c->jump.gravityFactor);
    float dragX = c->jump.dragXMult, dragY = c->jump.dragYMult, dragZ = c->jump.dragZMult, aoa = c->jump.aoaAdjustMult;
    c->flight = physics->tumble;
    c->flight.dragXMult = dragX;
    c->flight.dragYMult = dragY;
    c->flight.dragZMult = dragZ;
    c->flight.aoaAdjustMult = aoa;
    SoulCall::Handle(this, SQS_FLY, c, next);
    physics->physical.SetGravity(gravity);
}

// ---- the flight ----

// @00454b40 One frame of flight (gliding, diving, and every other kind of flight through the helpers above): steer, then the
// velocity changes by gravity, lift (along up, from the forward speed) and drag along each of the body's axes; flying backwards
// keeps only the sideways and vertical parts. A flight that reaches a surface lands there (SS_LAND), unless the surface's type
// pushes it back or runs a script instead. The pitch target makes the nose follow the velocity (angle of attack).
void SqueezilSoul::Fly(AirControl *c, int next)
{
    const FlightModel &f = c->flight;
    Contact landing;
    landing.face = -1;
    landing.second = 0;
    if (next != SS_NONE && SoulCall::CanEnter(this, next))
        SoulCall::SetState(this, next, 0);
    float seconds = FrameSeconds();
    float m[16];
    memcpy(m, BodyCall::Matrix(SoulCall::Body(this)), sizeof m);
    Vector3f position = Row(m, 3);
    float speed = BodyCall::Speed(SoulCall::Body(this));
    Vector3f direction = BodyCall::Direction(SoulCall::Body(this));
    // (the original copies the body's contact here and never uses it)
    SoulCall::Steer(this, c, pitchTarget, m, seconds);
    Vector3f right = Row(m, 0), up = Row(m, 1), forward = Row(m, 2);
    float pitch, yaw, roll;
    BodyCall::GetOrientation(SoulCall::Body(this), &pitch, &yaw, &roll);

    Vector3f velocity = direction * speed;
    float along = velocity.z * forward.z + velocity.y * forward.y + velocity.x * forward.x;
    float rise = velocity.z * up.z + velocity.y * up.y + velocity.x * up.x;
    float side = velocity.x * right.x + velocity.y * right.y + velocity.z * right.z;
    float sideSign = side < 0.0f ? -1.0f : 1.0f;
    Vector3f dragX = right * -(side * sideSign * f.dragXMult * sideSign);
    float riseSign = rise < 0.0f ? -1.0f : 1.0f;
    Vector3f dragY = up * -(rise * riseSign * f.dragYMult * riseSign);
    float alongSign = along < 0.0f ? -1.0f : 1.0f;
    float alongSize = alongSign * along;
    Vector3f dragZ = forward * -(alongSize * f.dragZMult * alongSign);
    float lift = f.liftMult * alongSize * alongSign;
    if (c->pitch != kNoInput)
        lift *= c->pitch > 0.0f ? f.tuckLiftFactor : f.climbLiftFactor;
    Vector3f liftForce = up * lift;
    Vector3f change = (liftForce + dragZ + dragY + dragX + physics->physical.gravity) * seconds;
    Vector3f newVelocity = velocity + change;

    float newAlong = forward.y * newVelocity.y + forward.z * newVelocity.z + forward.x * newVelocity.x;
    if (newAlong < 0.0f) {
        float a = right.y * newVelocity.y + right.z * newVelocity.z + right.x * newVelocity.x;
        rise = up.y * newVelocity.y + up.z * newVelocity.z + up.x * newVelocity.x;
        newVelocity = right * a + up * rise;
    }
    Vector3f newDirection = newVelocity;
    float newSpeed = Normalize(newDirection);
    Vector3f step = newVelocity * seconds * f.speedFactor;

    Float3 from = ToFloat3(position), r = ToFloat3(right), u = ToFloat3(up), fw = ToFloat3(forward), d = ToFloat3(newDirection);
    Float3 hit;
    int landed = FlightHitsSurface(GetPlaceSurfaces(place), &from, &r, &u, &fw, &d, Length(step), &hit, &r, &u, &fw, &landing);
    if (landed && NavSurfaceBlocksWalking(GetPlaceSurfaces(place), &landing)) {
        landed = 0;
        step = step * -1.0f;
    }
    if (landed && NavSurfaceHasTouchScript(GetPlaceSurfaces(place), &landing)) {
        landed = 0;
        RunScript(this, NavSurfaceTouchScript(GetPlaceSurfaces(place), &landing));
    }
    right = ToVector(r);
    up = ToVector(u);
    forward = ToVector(fw);
    if (landed) {
        float landedMatrix[16];
        Vector3f at = ToVector(hit);
        SetRows(landedMatrix, right, up, forward, at);
        BodyCall::SetContact(SoulCall::Body(this), &landing);
        BodyCall::SetMatrix(SoulCall::Body(this), landedMatrix);
        SoulCall::SetState(this, SS_LAND, 0);
        pitchTarget = kNoInput;
        CheckFloorLevel(this, &at, position.y);     // (after the matrix is set, so the hold has no effect)
        return;
    }
    Vector3f newPosition = position + step;
    CheckFloorLevel(this, &newPosition, position.y);
    float adjust;
    if (newSpeed > 0.01f) {
        adjust = -(rise / newSpeed) * f.aoaAdjustMult;
        if (SoulCall::GetState(this) == SS_FLY && newAlong < 0.0f)
            adjust = adjust - f.stallAdjustMult / newSpeed * newAlong;
    } else {
        adjust = 0.0f;
    }
    pitchTarget = adjust + pitch;
    SetTranslation(m, newPosition);
    BodyCall::SetMatrix(SoulCall::Body(this), m);
    BodyCall::SetVelocity(SoulCall::Body(this), newDirection, newSpeed);
}

// ---- the other state handlers ----

// @004558f0 Crouch (hop piece 1), hold (2) until the hop is let go (hopStrength set), leap (3): the hop's speeds times the
// strength. Piece 5 is the landing crouch, after which the speed picks the ground state. Meanwhile the squeezil walks.
void SqueezilSoul::HopStart(GroundControl *c, int next)
{
    switch (BodyCall::GetMode(SoulCall::Body(this))) {
    case MODE_HOP_1:
        if (BodyCall::ClipEnded(SoulCall::Body(this)))
            BodyCall::SetMode(SoulCall::Body(this), MODE_HOP_2);
        break;
    case MODE_HOP_2:
        if (hopStrength != -1.0f)
            BodyCall::SetMode(SoulCall::Body(this), MODE_HOP_3);
        break;
    case MODE_HOP_3:
        if (BodyCall::ClipEnded(SoulCall::Body(this))) {
            const JumpModel &hop = physics->hop;
            float up = hop.upSpeed * hopStrength, fwd = hop.fwdSpeed * hopStrength;
            hopStrength = -1.0f;
            SoulCall::StartJump(this, up, fwd, hop.upDistance);
            SoulCall::SetState(this, SS_HOP, 0);
            return;
        }
        break;
    case MODE_HOP_5:
        if (next == SS_STAND) {
            SqueezilBody *b = SoulCall::Body(this);
            BodyCall::SetVelocity(b, BodyCall::Direction(SoulCall::Body(this)), 0.0f);
        }
        if (BodyCall::ClipEnded(SoulCall::Body(this))) {
            if (next == SS_NONE)
                next = SoulCall::GroundStateForSpeed(this, BodyCall::Speed(SoulCall::Body(this)));
            if (!SoulCall::CanEnter(this, next))
                next = SS_STAND;
            SoulCall::SetState(this, next, 0);
            SoulCall::Handle(this, SQS_GROUND_MOVE, c, next);
            return;
        }
        break;
    }
    c->ground = physics->walk;
    SoulCall::Handle(this, SQS_GROUND_MOVE, c, SS_NONE);
}

// @00455f10 Spitting while standing: the mouth opens (spit piece 1, then 2) and waits for the spit to be let go (spitStrength
// set), then piece 3 and the nut (SpitVelocity times the strength, at least 0.2). The squeezil may walk meanwhile.
void SqueezilSoul::SpitStanding(GroundControl *c, int next)
{
    int mode = BodyCall::GetMode(SoulCall::Body(this));
    c->ground = physics->walk;
    SoulCall::Handle(this, SQS_GROUND_MOVE, c, SS_NONE);
    switch (spitPhase) {
    case 0:
        spitPhase = 1;
        // fall through
    case 1:
        if (BodyCall::ClipEnded(SoulCall::Body(this))) {
            spitPhase++;
            BodyCall::SetMode(SoulCall::Body(this), MODE_SPIT_2);
        }
        break;
    case 3:
        if (mode == MODE_SPIT_3 && BodyCall::ClipEnded(SoulCall::Body(this))) {
            SoulCall::SetState(this, next != SS_NONE && SoulCall::CanEnter(this, next) ? next : SS_STAND, 1);
            spitPhase = 0;
            return;
        }
        break;
    }
    if (spitStrength != -1.0f) {
        BodyCall::SetMode(SoulCall::Body(this), MODE_SPIT_3, 1);
        if (spitStrength < 0.2f)
            spitStrength = 0.2f;
        PlayEffect("spitEffect", BodyPosition(this));
        SoulCall::SpitNut(this, physics->spitVelocity * spitStrength);
        spitStrength = -1.0f;
        spitPhase = 3;
    }
}

// @004560d0 The sliding attack: a push (slide piece 1, PushIncrement more speed), the slide (piece 2) with another push whenever
// the attack is still held and the speed fell below MinSpeed, and the stop (piece 3). It moves like a slip throughout.
void SqueezilSoul::Attack(GroundControl *c, int next)
{
    const LeverMotion &l = c->lever;
    switch (BodyCall::GetMode(SoulCall::Body(this))) {
    case MODE_SLIDE_1:
        if (BodyCall::ClipEnded(SoulCall::Body(this))) {
            if (frozen == 0) {
                float speed = BodyCall::Speed(SoulCall::Body(this));
                Vector3f direction = BodyCall::Direction(SoulCall::Body(this));
                Vector3f forward = Row(BodyCall::Matrix(SoulCall::Body(this)), 2);
                Vector3f velocity = direction * speed + forward * l.pushIncrement;
                float newSpeed = Normalize(velocity);
                BodyCall::SetVelocity(SoulCall::Body(this), velocity, newSpeed);
            }
            BodyCall::SetMode(SoulCall::Body(this), MODE_SLIDE_2);
        }
        break;
    case MODE_SLIDE_2:
        if (InputHeldAttack() && frozen == 0) {
            if (BodyCall::Speed(SoulCall::Body(this)) < l.minSpeed)
                BodyCall::SetMode(SoulCall::Body(this), MODE_SLIDE_1);
        } else {
            BodyCall::SetMode(SoulCall::Body(this), MODE_SLIDE_3);
        }
        break;
    case MODE_SLIDE_3:
        if (BodyCall::ClipEnded(SoulCall::Body(this))) {
            int to = next == SS_SKID ? SS_SKID : SoulCall::GroundStateForSpeed(this, BodyCall::Speed(SoulCall::Body(this)));
            SoulCall::SetState(this, to, 1);
            return;
        }
        break;
    }
    SoulCall::Handle(this, SQS_SLIP, c, SS_NONE);
}

// @00456350 Knocked over: it slides (stumble pieces 1 and 2) until it stops, then gets up (piece 3) and stands.
void SqueezilSoul::Stumble(GroundControl *c, int next)
{
    int mode = BodyCall::GetMode(SoulCall::Body(this));
    if (mode == MODE_STUMBLE_3) {
        if (BodyCall::ClipEnded(SoulCall::Body(this))) {
            SqueezilBody *b = SoulCall::Body(this);
            BodyCall::SetVelocity(b, BodyCall::Direction(SoulCall::Body(this)), 0.0f);
            SoulCall::SetState(this, SS_STAND, 1);
            directions.Clear();
        }
        return;
    }
    if (mode != MODE_STUMBLE_1 && mode != MODE_STUMBLE_2)
        return;
    if (mode == MODE_STUMBLE_1 && BodyCall::ClipEnded(SoulCall::Body(this)))
        BodyCall::SetMode(SoulCall::Body(this), MODE_STUMBLE_2);
    SoulCall::Handle(this, SQS_SLIP, c, SS_STAND);
    if (SoulCall::GetState(this) == SS_STAND) {          // the slide stopped: back to stumbling, to get up
        SoulCall::SetState(this, SS_STUMBLE, 0);
        BodyCall::SetMode(SoulCall::Body(this), MODE_STUMBLE_3);
    }
}

// @00456430 Lies down (sleep piece 1), then sleeps (piece 2) until anything is wished for or it is turned.
void SqueezilSoul::LieDown(GroundControl *c, int next)
{
    if (BodyCall::GetMode(SoulCall::Body(this)) == MODE_SLEEP_1 && BodyCall::ClipEnded(SoulCall::Body(this)))
        BodyCall::SetMode(SoulCall::Body(this), MODE_SLEEP_2);
    if ((next == SS_NONE || next == SS_STAND) && !(fabsf(c->turn) > 0.1f))
        return;
    SoulCall::SetState(this, SS_GET_UP, 0);
    if (*kLoopingSoundPlaying)
        StopLoopingSounds();
}

// @004564b0
void SqueezilSoul::GetUp(GroundControl *c, int next)
{
    if (BodyCall::GetMode(SoulCall::Body(this)) == MODE_SLEEP_3 && BodyCall::ClipEnded(SoulCall::Body(this))) {
        SoulCall::SetState(this, SS_STAND, 0);
        if (next != SS_NONE)
            SoulCall::SetState(this, next, 0);
    }
}

// @004565e0 Touched down: going forward it keeps the forward part of its speed (at most MaxLandingSpeed, unless it tumbled in,
// and MaxRunSpeed when running) and picks walk, run or stumble from it; going sideways it slips; else it stops.
void SqueezilSoul::Land(GroundControl *c, int next)
{
    float speed = BodyCall::Speed(SoulCall::Body(this));
    Vector3f direction = BodyCall::Direction(SoulCall::Body(this));
    Vector3f right = Row(BodyCall::Matrix(SoulCall::Body(this)), 0);
    Vector3f forward = Row(BodyCall::Matrix(SoulCall::Body(this)), 2);
    if (speed > 0.01f) {
        float along = forward.y * direction.y + forward.z * direction.z + forward.x * direction.x;
        float sideways = right.y * direction.y + right.z * direction.z + right.x * direction.x;
        int to;
        if (along > (double)0.3) {
            speed = along * speed;
            to = next == SS_SKID ? SS_SKID : SoulCall::GroundStateForSpeed(this, speed);
        } else if (sideways > (double)0.3) {
            speed = along * speed;
            to = SS_SLIP;
        } else {
            to = SS_STAND;
            speed = 0.0f;
        }
        SoulCall::SetState(this, to, 0);
        if (physics->maxLandingSpeed < speed && limitLanding)
            speed = physics->maxLandingSpeed;
        limitLanding = -1;
        if (to == SS_RUN && physics->maxRunSpeed < speed)
            speed = physics->maxRunSpeed;
        BodyCall::SetVelocity(SoulCall::Body(this), forward, speed);
    } else {
        SoulCall::SetState(this, SS_STAND, 1);
        BodyCall::SetVelocity(SoulCall::Body(this), direction, 0.0f);
        speed = 0.0f;
    }
    BodyCall::SetClipSpeed(SoulCall::Body(this), speed);
}

// @00456500
void SqueezilSoul::Dying(GroundControl *c, int next)
{
    if (BodyCall::ClipEnded(SoulCall::Body(this))) {
        finished = 1;
        SoulCall::SetState(this, SS_DEAD, 0);
    }
}

// @00456800 Braking: it moves with no forward wish and without slipping; when the brake clip is near standstill the skid sound
// plays (skid piece 2), then it stops. A backward wish keeps it braking.
void SqueezilSoul::Skid(GroundControl *c, int next)
{
    int reversing = c->forward < 0.0f;
    c->forward = 0.0f;
    canSlip = 0;
    SoulCall::Handle(this, SQS_GROUND_MOVE, c, SS_NONE);
    canSlip = 1;
    if (SoulCall::GetState(this) == SS_SLIP)
        return;
    switch (BodyCall::GetMode(SoulCall::Body(this))) {
    case MODE_STOP:
        if (!reversing) {
            if (next != SS_NONE && SoulCall::CanEnter(this, next))
                SoulCall::SetState(this, next, 0);
            else
                SoulCall::SetState(this, SS_STAND, 0);
        }
        break;
    case MODE_SKID_1:
        if (!(fabsf(BodyCall::Speed(SoulCall::Body(this))) > 0.01f)) {
            PlayEffect("skidEffect", BodyPosition(this));
            BodyCall::SetMode(SoulCall::Body(this), MODE_SKID_2);
        }
        break;
    case MODE_SKID_2:
        if (BodyCall::ClipEnded(SoulCall::Body(this)))
            BodyCall::SetMode(SoulCall::Body(this), MODE_STOP);
        break;
    }
}

// @004558a0
void SqueezilSoul::JumpAir(AirControl *c, int next)
{
    if (BodyCall::GetMode(SoulCall::Body(this)) == MODE_JUMP_1 && BodyCall::ClipEnded(SoulCall::Body(this)))
        BodyCall::SetMode(SoulCall::Body(this), MODE_JUMP_2);
    JumpFlight(c, next);
}

// @00455a90 A hop that lands crouches for the next hop (hop piece 5), keeping the forward part of its speed (at most
// MaxLandingSpeed).
void SqueezilSoul::HopAir(AirControl *c, int next)
{
    JumpFlight(c, next);
    if (SoulCall::GetState(this) != SS_LAND)
        return;
    SoulCall::SetState(this, SS_HOP_START, 0);
    BodyCall::SetMode(SoulCall::Body(this), MODE_HOP_5, 1);
    float speed = BodyCall::Speed(SoulCall::Body(this));
    Vector3f direction = BodyCall::Direction(SoulCall::Body(this));
    Vector3f forward = Row(BodyCall::Matrix(SoulCall::Body(this)), 2);
    if (physics->maxLandingSpeed < speed)
        speed = physics->maxLandingSpeed;
    speed = (forward.y * direction.y + forward.z * direction.z + forward.x * direction.x) * speed;
    BodyCall::SetVelocity(SoulCall::Body(this), forward, speed);
    SoulCall::UpdateSurface(this, *BodyCall::GetContact(SoulCall::Body(this)));
}

// @00455c00
void SqueezilSoul::SkihopAir(AirControl *c, int next)
{
    JumpFlight(c, next);
    if (SoulCall::GetState(this) == SS_LAND)
        SoulCall::UpdateSurface(this, *BodyCall::GetContact(SoulCall::Body(this)));
}

// @00454ab0 Tumbling: the tumble flight model with the nose pushed down a little, until TumbleDuration is over (then it glides).
void SqueezilSoul::Tumble(AirControl *c, int next)
{
    tumbleTime -= FrameSeconds();
    if (!(tumbleTime > 0.0f)) {
        SoulCall::SetState(this, SS_FLY, 0);
        return;
    }
    c->flight = physics->tumble;
    c->pitch = 0.5f;
    c->turn = 0.0f;
    SoulCall::Handle(this, SQS_FLY, c, next);
}

// @00456530 Falls with the flight model of the state it died in; on touching down it is dead.
void SqueezilSoul::DyingInAir(AirControl *c, int next)
{
    SoulCall::GetAirMotion(this, stateAtDeath, c);
    c->pitch = kNoInput;
    c->turn = 0.0f;
    SoulCall::Handle(this, SQS_FLY, c, SS_NONE);
    if (SoulCall::GetState(this) == SS_LAND) {
        SoulCall::SetState(this, SS_DYING, 0);
        SoulCall::SetState(this, SS_DEAD, 0);
    }
}
