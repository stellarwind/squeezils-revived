// CameraSoul, RegionCameraSoul, PlaceCameraSoul: see CameraSoul.h. Reconstructed from SQZ.EXE 0x408600-0x40b840.
#include "CameraSoul.h"
#include "SqueezilSoulInternal.h"
#include "CameraBody.h"
#include "SqueezilBody.h"
#include "BodyPart.h"
#include "Animation.h"
#include "Scene.h"
#include "IniReader.h"
#include "Original.h"
#include "VirtualCall.h"
#include <string.h>
#include <new>
#include <math.h>

static_assert(offsetof(CameraSoul, povOffset) == 0x18, "CameraSoul layout");
static_assert(offsetof(CameraSoul, fromMatrix) == 0x88, "CameraSoul layout");
static_assert(offsetof(CameraSoul, toMatrix) == 0x108, "CameraSoul layout");
static_assert(offsetof(CameraSoul, lookMode) == 0x148, "CameraSoul layout");
static_assert(offsetof(CameraSoul, path) == 0x160, "CameraSoul layout");

static CameraSettings &gRegionSettings = *(CameraSettings *)0x4c64e8;
static CameraSettings &gPlaceSettings  = *(CameraSettings *)0x4c6488;
static int   &gRegionSettingsRead = *(int *)0x4b9888;
static int   &gPlaceSettingsRead  = *(int *)0x4b988c;
static float &gInterpTime         = *(float *)0x4c6480;      // [Camera] CamInterpTime (0.5): the length of a blend

static void **const kMessageMapVtable   = (void **)0x49d2b4;
static void **const kSoulVtable         = (void **)0x49d2a4;
static void **const kInputTargetVtable  = (void **)0x49d2c4;
static void **const kSimulationSoulVtable = (void **)0x49d2ec;
static void **const kCameraSoulVtable   = (void **)0x49d33c;
static void **const kCameraInputVtable  = (void **)0x49d314;
static void **const kRegionCameraVtable = (void **)0x49d3c4;
static void **const kRegionInputVtable  = (void **)0x49d39c;
static void **const kPlaceCameraVtable  = (void **)0x49d414;
static void **const kPlaceInputVtable   = (void **)0x49d3ec;

static const float kPi180 = 0.0174533f;     // 0x49d37c

static void SetIdentity(float *m)
{
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// A matrix that only moves
static void SetShift(float *m, const Vector3f &by)
{
    SetIdentity(m);
    m[12] = by.x;
    m[13] = by.y;
    m[14] = by.z;
}

static int Bits(float f)
{
    int i;
    memcpy(&i, &f, sizeof i);
    return i;
}

// The body behind the camera is reached through its own table (it is an original object)
static float *TargetMatrix(CameraSoul *s) { return CallVirtual<float *>(s->target, BODY_GET_MATRIX); }
static int    TargetOnGround(CameraSoul *s) { return CallVirtual<int>(s->target, SQB_ON_GROUND); }
static int    TargetState(CameraSoul *s) { return CallVirtual<int>(s->targetSoul, SQS_GET_STATE); }
static void   SetCameraMatrix(CameraSoul *s, const float *m) { CallVirtual<void>(s->camera, BODY_SET_MATRIX, m); }

// @00408600
void ReadCameraSettings(const char *section, CameraSettings *out)
{
    IniReader ini;
    ini.SetSection(section);
    out->povOffset.x = ini.GetFloat("PovOffsetX", 0.0f);
    out->povOffset.y = ini.GetFloat("PovOffsetY", 0.0f);
    out->povOffset.z = ini.GetFloat("PovOffsetZ", 0.0f);
    out->autoLeashMode = ini.GetInt("AutoLeashMode", 1);
    out->lookAtReturnMode = ini.GetInt("LookAtReturnMode", 1);
    out->leashLength = ini.GetFloat("LeashLength", 1.0f);
    out->outLeashSpeed = ini.GetFloat("OutLeashSpeed", 0.0f);
    out->povLeashSpeed = ini.GetFloat("PovLeashSpeed", 2.0f);
    out->cameraOutPitch = ini.GetFloat("CameraOutPitch", 0.0f);
    out->cameraPovPitch = ini.GetFloat("CameraPovPitch", 0.0f);
    out->cameraOutYaw = ini.GetFloat("CameraOutYaw", 0.0f);
    out->cameraPovYaw = ini.GetFloat("CameraPovYaw", 0.0f);
    out->lookAtAttenPitch = ini.GetFloat("LookAtAttenPitch", 0.0f);
    out->lookAtAttenYaw = ini.GetFloat("LookAtAttenYaw", 0.0f);
    out->lookAtPitchRange = ini.GetFloat("LookAtPitchRange", 0.0f);
    out->lookAtYawRange = ini.GetFloat("LookAtYawRange", 0.0f);
    out->lookAtTurnValue = ini.GetFloat("LookAtTurnValue", 4.0f);
    out->returnToBaseTime = ini.GetFloat("ReturnToBaseTime", 0.4f);
    out->flyPitchRange = ini.GetFloat("FlyPitchRange", 0.0f);
    out->flyYawRange = ini.GetFloat("FlyYawRange", 0.0f);
    out->maxJumpHeight = ini.GetFloat("MaxJumpHeight", 0.0f);
    out->maxFlyPitchAdjust = ini.GetFloat("MaxFlyPitchAdjust", 0.0f);
    out->maxFlyYawAdjust = ini.GetFloat("MaxFlyYawAdjust", 0.0f);
    out->maxJumpPitchAdjust = ini.GetFloat("MaxJumpPitchAdjust", 0.0f);
    ini.SetSection("Camera");
    gInterpTime = ini.GetFloat("CamInterpTime", 0.5f);
}

// @00408850 The camera's matrix for a target matrix.
//   A  the target's matrix with the camera's turning point (povOffset) as its origin
//   B  a turn about that point by the pitch, C one by the yaw (both about the point in the target's frame)
//   F  = shift(0, 0, -leash) * B * C * A: the camera's place; A's position is then replaced by F's
//   result = pitch * yaw * A: the camera looks the way the target does, turned by the pitch and the yaw less what the look-at
//   attenuation takes off (a flight or a hop tips the view the other way).
// Where the original leaves a value unset (the jump fraction when MaxJumpHeight is 0), ours uses 0.
Matrix16 CameraSoul::ComputeView(const CameraSettings *s, Matrix16 targetMatrix)
{
    float speed = CallVirtual<float>(target, BODY_GET_FACING_VALUE);
    float lookPitchAtten = 0.0f;            // [ebp-0x14]
    float lookYawAtten = 0.0f;              // [ebp-0x10]
    float jumpFraction = 0.0f;

    float shift[16], placed[16];
    SetShift(shift, povOffset);
    MultiplyMatrix(placed, shift, targetMatrix.m);                              // A

    // how far "out" the camera is: a squeezil that goes faster pulls it to the "pov" values
    if (TargetOnGround(this) && autoLeashMode) {
        int st = TargetState(this);
        if (st != SS_LAND && st != SS_STUMBLE && st != SS_HOP_START && st != SS_STAND) {
            if (s->outLeashSpeed > speed)
                leashBlend = 1.0f;
            else if (s->povLeashSpeed > speed)
                leashBlend = ((s->povLeashSpeed - speed) / (s->povLeashSpeed - s->outLeashSpeed)) * leashScale;
            else
                leashBlend = 0.0f;
        } else {
            leashBlend = 1.0f;
        }
    } else {
        leashBlend = 1.0f;
    }

    float leash = leashLength * (autoLeashMode ? leashBlend : leashScale);
    float back[16];
    SetShift(back, Vector3f(0.0f, 0.0f, -leash));
    if (lookAtReturnMode) {
        lookPitch = lookPitch * leashBlend;
        lookYaw = lookYaw * leashBlend;
    }

    float toPoint[16], fromPoint[16];
    SetShift(toPoint, povOffset * -1.0f);                                       // M3: the turning point to the origin
    SetShift(fromPoint, povOffset);                                             // M4: and back

    // the player looks about with the mouse
    if (lookMode == 0) {
        float lookA, lookB;
        ApplicationLookDelta(GetApplication(), &lookA, &lookB);
        lookPitch = s->lookAtTurnValue * lookA + lookPitch;
        lookYaw = s->lookAtTurnValue * lookB + lookYaw;
        if (!(lookPitch < s->lookAtPitchRange))
            lookPitch = s->lookAtPitchRange;
        else if (!(Bits(lookPitch) > 0))
            lookPitch = 0.0f;
    }

    float pitch = (s->cameraPovPitch - s->cameraOutPitch) * leashBlend + lookPitch + s->cameraOutPitch;
    if (!TargetOnGround(this)) {
        int st = TargetState(this);
        if (st == SS_FLY) {
            if (Bits(s->flyPitchRange) > 0) {
                if (!(CallVirtual<float>(target, SQB_GET_PITCH) < 0.0f)) {
                    float t = CallVirtual<float>(target, SQB_GET_PITCH) / s->flyPitchRange;
                    t = (t < 0.0f) ? 0.0f : (1.0f > t ? t : 1.0f);
                    pitch = s->maxFlyPitchAdjust * t + pitch;
                }
            }
        } else if (st == SS_HOP) {
            if (Bits(s->maxJumpHeight) > 0) {
                float height = TargetMatrix(this)[13] - targetSoul->jumpStartY;
                float t = height / s->maxJumpHeight;
                jumpFraction = (t < 0.0f) ? 0.0f : (1.0f > t ? t : 1.0f);
                pitch = s->maxJumpPitchAdjust * jumpFraction + pitch;
            }
        }
    }
    float pitchRadians = pitch * kPi180;
    float turn[16], stepA[16], stepB[16], pitchTurn[16], yawTurn[16];
    RotationX(turn, pitchRadians);
    MultiplyMatrix(stepA, fromPoint, turn);
    MultiplyMatrix(pitchTurn, stepA, toPoint);                                  // B

    float yaw = (lookMode == 0) ? (s->cameraPovYaw - s->cameraOutYaw) * leashBlend + lookYaw + s->cameraOutYaw : lookYaw;
    float yawRadians = yaw * kPi180;
    RotationY(turn, yawRadians);
    MultiplyMatrix(stepB, fromPoint, turn);
    MultiplyMatrix(yawTurn, stepB, toPoint);                                    // C

    float t1[16], t2[16], whole[16];
    MultiplyMatrix(t1, yawTurn, placed);
    MultiplyMatrix(t2, pitchTurn, t1);
    MultiplyMatrix(whole, back, t2);                                            // F
    SetTranslation(placed, Row(whole, 3));

    if (TargetOnGround(this)) {
        lookPitchAtten = sinf(pitchRadians) * s->lookAtAttenPitch;
        lookYawAtten = sinf(yawRadians) * s->lookAtAttenYaw;
    }
    if (!TargetOnGround(this)) {
        int st = TargetState(this);
        if (st == SS_FLY) {
            float roll = 0.0f;
            if (Bits(s->flyYawRange) > 0) {
                float r = CallVirtual<float>(target, SQB_GET_ROLL) / s->flyYawRange;
                roll = (r < -1.0f) ? -1.0f : (1.0f > r ? r : 1.0f);
            }
            lookYawAtten = s->maxFlyYawAdjust * roll + lookYawAtten;
        } else if (st == SS_HOP) {
            lookPitchAtten = lookPitchAtten - s->maxJumpPitchAdjust * jumpFraction;
        }
    }

    float lookX[16], lookY[16], lookXY[16];
    RotationX(lookX, (pitch - lookPitchAtten) * kPi180);
    RotationY(lookY, (yaw - lookYawAtten) * kPi180);
    MultiplyMatrix(lookXY, lookY, placed);
    Matrix16 result;
    MultiplyMatrix(result.m, lookX, lookXY);
    return result;
}

// @00409620 The matrix a blend has got to: the right, up and forward rows by spherical interpolation, the position by linear
// interpolation (blendPosition) or the target's at once. A row that has shrunk to nothing is replaced by an axis.
Matrix16 CameraSoul::BlendMatrices()
{
    float t = blendTime / blendDuration;
    Vector3f position;
    if (blendPosition) {
        position = Row(toMatrix, 3) * t;
        position = Row(fromMatrix, 3) * (1.0f - t) + position;
    } else {
        position = Row(toMatrix, 3);
    }
    Vector3f right = Slerp(t, Row(fromMatrix, 0), Row(toMatrix, 0));
    Vector3f up = Slerp(t, Row(fromMatrix, 1), Row(toMatrix, 1));
    Vector3f forward = Slerp(t, Row(fromMatrix, 2), Row(toMatrix, 2));
    if (right.x * right.x + right.y * right.y + right.z * right.z < 1.0e-4f)
        right = Vector3f(1.0f, 0.0f, 0.0f);
    if (up.x * up.x + up.y * up.y + up.z * up.z < 1.0e-4f)
        up = Vector3f(0.0f, 1.0f, 0.0f);
    if (forward.x * forward.x + forward.y * forward.y + forward.z * forward.z < 1.0e-4f)
        forward = Vector3f(0.0f, 0.0f, 1.0f);
    Matrix16 result;
    SetRows(result.m, right, up, forward, position);
    OrthonormalizeMatrix(result.m);
    return result;
}

// @00409a50
void CameraSoul::StartPath(MorphCycle *clip, int loopMode, int mode, float duration)
{
    if (!clip) {
        GameLog("Path is not valid.\n");
        return;
    }
    CallVirtual<void>(targetSoul, SQS_FREEZE);
    BodyPart *part = (BodyPart *)Operator_New(sizeof(BodyPart));
    if (part)
        part = part->Init();
    path = (AnimatedBody *)Operator_New(sizeof(AnimatedBody));
    if (path)
        path = path->Init();
    path->AddPart("part", part);
    path->Build(GetScene()->GetSceneChild());
    path->SetCycle(clip, 0);
    path->SetTime(0.0f);
    path->SetLoopMode(loopMode);
    pathMode = mode;
    enabled = 0;
    blendTime = 0.0f;
    blendDuration = duration;
    memcpy(fromMatrix, CallVirtual<float *>(camera, BODY_GET_MATRIX), sizeof fromMatrix);
    blending = 1;
    blendPosition = 1;
    lookMode = 1;
}

// @00409c40
void CameraSoul::StopPath(float duration)
{
    enabled = 1;
    blendTime = 0.0f;
    blendDuration = duration;
    memcpy(fromMatrix, CallVirtual<float *>(camera, BODY_GET_MATRIX), sizeof fromMatrix);
    blending = 1;
    blendPosition = 1;
    lookMode = 1;
}

// the first six words of the settings become the soul's own (0x18 to 0x2c)
void CameraSoul::CopySettingsHead(const CameraSettings &s)
{
    povOffset = Vector3f(s.povOffset.x, s.povOffset.y, s.povOffset.z);
    autoLeashMode = s.autoLeashMode;
    lookAtReturnMode = s.lookAtReturnMode;
    leashLength = s.leashLength;
}

// the part of both Updates that notices the target's up vector change (the previous matrix is the body's lastMatrix)
void CameraSoul::StartBlend()
{
    blendTime = 0.0f;
    blendDuration = gInterpTime;
    memcpy(fromMatrix, target->lastMatrix, sizeof fromMatrix);
    blending = 1;
    blendPosition = 0;
}

// the mouse-look coming back to rest (lookMode 1: work out the rates, 2: apply them until both angles are 0)
void CameraSoul::ReturnLook(float returnToBaseTime)
{
    if (lookMode <= 0)
        return;
    if (lookMode == 1) {
        lookPitch = (float)((int)lookPitch % 360);
        lookYaw = (float)((int)lookYaw % 360);
        if (!(fabsf(lookYaw) <= 180.0f)) {
            float sign = ((unsigned)Bits(lookYaw) > 0x80000000u) ? -1.0f : 1.0f;
            lookYaw = lookYaw - sign * 360.0f;
        }
        returnPitchRate = -(lookPitch / returnToBaseTime);
        returnYawRate = -(lookYaw / returnToBaseTime);
        lookMode = 2;
    }
    if (lookMode == 2) {
        if (Bits(lookPitch) > 0) {
            lookPitch = returnPitchRate + lookPitch;
            if (!(Bits(lookPitch) > 0))
                lookPitch = 0.0f;
        }
        float yawSign = 1.0f;
        lookYaw = lookYaw + returnYawRate;
        if ((unsigned)Bits(lookYaw) > 0x80000000u)
            yawSign = -1.0f;
        float rateSign = 1.0f;
        if ((unsigned)Bits(returnYawRate) > 0x80000000u)
            rateSign = -1.0f;
        if (yawSign == rateSign)
            lookYaw = 0.0f;
        if ((Bits(lookPitch) & 0x7fffffff) == 0 && (Bits(lookYaw) & 0x7fffffff) == 0)
            lookMode = 0;
    }
}

// ---- RegionCameraSoul ----

// @00409d10 The camera stands behind the player at once; the first frame does not look for a change of up.
RegionCameraSoul *RegionCameraSoul::Init(CameraBody *cameraArg, SqueezilBody *targetArg, SqueezilSoul *soulArg)
{
    vtbl = kRegionCameraVtable;
    camera = (CameraBody *)cameraArg;
    target = targetArg;
    inputVtbl = kRegionInputVtable;
    targetSoul = soulArg;
    if (!gRegionSettingsRead) {
        ReadCameraSettings("RegionCamera", &gRegionSettings);
        gRegionSettingsRead = 1;
    }
    CopySettingsHead(gRegionSettings);
    lookPitch = 0.0f;
    leashScale = 1.0f;
    lookYaw = 0.0f;
    Matrix16 start;
    memcpy(start.m, TargetMatrix(this), sizeof start.m);
    Matrix16 view = ComputeView(&gRegionSettings, start);
    SetCameraMatrix(this, view.m);
    justStarted = 1;
    blending = 0;
    lookMode = 0;                   // (the original leaves this field as the memory had it)
    return this;
}

// @0040b7c0 slot 0 (the base classes' destructors only put back their own vtables)
RegionCameraSoul *RegionCameraSoul::DestroyAndFree(int flags)
{
    vtbl = kMessageMapVtable;
    inputVtbl = kMessageMapVtable;
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @0040a5e0 (this = the soul) The key to look back (0xe): the camera returns to its leash length behind the squeezil
void RegionCameraSoul::OnEvent(int what, const char *)
{
    if (what == 0xe && autoLeashMode && TargetState(this) != SS_LAND) {
        lookMode = 1;
        leashLength = gRegionSettings.leashLength;
    }
}

// @00409f70 slot 5
void RegionCameraSoul::UpdateRegion()
{
    if (!justStarted) {
        float *m = TargetMatrix(this);
        const float *last = target->lastMatrix;
        if (!(last[4] == m[4] && last[5] == m[5] && last[6] == m[6]))
            StartBlend();
    }
    ReturnLook(gRegionSettings.returnToBaseTime);

    Matrix16 view;
    if (blending) {
        blendTime = (float)FrameMilliseconds() * 0.001f + blendTime;
        if (!(blendTime < gInterpTime)) {
            blending = 0;
            Matrix16 to;
            memcpy(to.m, toMatrix, sizeof to.m);
            view = ComputeView(&gRegionSettings, to);
        } else {
            memcpy(toMatrix, TargetMatrix(this), sizeof toMatrix);
            view = ComputeView(&gRegionSettings, BlendMatrices());
        }
    } else {
        Matrix16 now;
        memcpy(now.m, TargetMatrix(this), sizeof now.m);
        view = ComputeView(&gRegionSettings, now);
        if (justStarted == 1)
            justStarted = 0;
    }
    SetCameraMatrix(this, view.m);
}

// ---- PlaceCameraSoul ----

// @0040a630
PlaceCameraSoul *PlaceCameraSoul::Init(CameraBody *cameraArg, SqueezilBody *targetArg, SqueezilSoul *soulArg)
{
    vtbl = kPlaceCameraVtable;
    camera = (CameraBody *)cameraArg;
    target = targetArg;
    targetSoul = soulArg;
    enabled = 1;
    inputVtbl = kPlaceInputVtable;
    if (!gPlaceSettingsRead) {
        ReadCameraSettings("PlaceCamera", &gPlaceSettings);
        gPlaceSettingsRead = 1;
    }
    CopySettingsHead(gPlaceSettings);
    lookPitch = 0.0f;
    leashScale = 1.0f;
    lookYaw = 0.0f;
    Matrix16 start;
    memcpy(start.m, TargetMatrix(this), sizeof start.m);
    Matrix16 view = ComputeView(&gPlaceSettings, start);
    SetCameraMatrix(this, view.m);
    blending = 0;
    lookMode = 0;
    justStarted = 1;
    return this;
}

// @0040b830 slot 0
PlaceCameraSoul *PlaceCameraSoul::DestroyAndFree(int flags)
{
    vtbl = kMessageMapVtable;
    inputVtbl = kMessageMapVtable;
    if (flags & 1)
        Operator_Delete(this);
    return this;
}

// @0040b2f0 (this = the soul)
void PlaceCameraSoul::OnEvent(int what, const char *)
{
    if (enabled && what == 0xe && autoLeashMode && TargetState(this) != SS_LAND) {
        lookMode = 1;
        leashLength = gPlaceSettings.leashLength;
    }
}

// @0040a820 slot 5
void PlaceCameraSoul::UpdatePlace()
{
    Matrix16 view;
    if (enabled) {
        if (TargetOnGround(this) && !justStarted) {
            float *m = TargetMatrix(this);
            const float *last = target->lastMatrix;
            if (!(last[4] == m[4] && last[5] == m[5] && last[6] == m[6]))
                StartBlend();
        }
        ReturnLook(gPlaceSettings.returnToBaseTime);

        if (blending) {
            blendTime = (float)FrameMilliseconds() * 0.001f + blendTime;
            if (!(blendTime < blendDuration)) {
                blending = 0;
                CallVirtual<void>(targetSoul, SQS_UNFREEZE);
                Matrix16 to;
                memcpy(to.m, toMatrix, sizeof to.m);
                view = ComputeView(&gPlaceSettings, to);
            } else {
                memcpy(toMatrix, TargetMatrix(this), sizeof toMatrix);
                view = ComputeView(&gPlaceSettings, BlendMatrices());
            }
        } else {
            Matrix16 now;
            memcpy(now.m, TargetMatrix(this), sizeof now.m);
            view = ComputeView(&gPlaceSettings, now);
            if (justStarted == 1)
                justStarted = 0;
        }
        SetCameraMatrix(this, view.m);
        return;
    }

    // a path is being played: the camera follows its "part"
    path->Advance((float)FrameMilliseconds() * 0.001f, 0);
    float animated[16];
    path->pose->Find("part")->ToMatrix(animated);
    Matrix16 wanted;
    switch (pathMode) {
    case 1:
        memcpy(wanted.m, animated, sizeof wanted.m);
        break;
    case 2:
        memcpy(wanted.m, animated, sizeof wanted.m);
        SetTranslation(wanted.m, Row(animated, 3) + Row(TargetMatrix(this), 3));
        break;
    default:
        MultiplyMatrix(wanted.m, animated, TargetMatrix(this));
        break;
    }

    if (blending) {
        blendTime = (float)FrameMilliseconds() * 0.001f + blendTime;
        if (!(blendTime < blendDuration)) {
            blending = 0;
            view = wanted;
        } else {
            memcpy(toMatrix, wanted.m, sizeof toMatrix);
            view = BlendMatrices();
        }
        SetCameraMatrix(this, view.m);
    } else {
        SetCameraMatrix(this, wanted.m);
    }
}
