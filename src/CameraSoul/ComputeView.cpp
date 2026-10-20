// SQZ.EXE 0x408850: camera leash, look input and flight/jump view adjustment.
#include <math.h>
#include "compat41.h"
struct ViewVector {
    float x, y, z;
    ViewVector() {}
    ViewVector(float a, float b, float c) : x(a), y(b), z(c) {}
    ~ViewVector();                                   // 0x401660
    ViewVector operator*(float n) const;             // 0x401b60
};
struct ViewMatrix {
    float m[16];
    ViewMatrix() {}
    ViewMatrix(const ViewMatrix &o) {
        m[0]=o.m[0]; m[1]=o.m[1]; m[2]=o.m[2]; m[3]=o.m[3];
        m[4]=o.m[4]; m[5]=o.m[5]; m[6]=o.m[6]; m[7]=o.m[7];
        m[8]=o.m[8]; m[9]=o.m[9]; m[10]=o.m[10]; m[11]=o.m[11];
        m[12]=o.m[12]; m[13]=o.m[13]; m[14]=o.m[14]; m[15]=o.m[15];
    }
    ~ViewMatrix();                                   // 0x402310
    ViewMatrix &operator=(const ViewMatrix &o) {
        m[0]=o.m[0]; m[1]=o.m[1]; m[2]=o.m[2]; m[3]=o.m[3];
        m[4]=o.m[4]; m[5]=o.m[5]; m[6]=o.m[6]; m[7]=o.m[7];
        m[8]=o.m[8]; m[9]=o.m[9]; m[10]=o.m[10]; m[11]=o.m[11];
        m[12]=o.m[12]; m[13]=o.m[13]; m[14]=o.m[14]; m[15]=o.m[15];
        return *this;
    }
    void Shift(const ViewVector &v) {
        m[0]=1; m[1]=0; m[2]=0; m[3]=0;
        m[4]=0; m[5]=1; m[6]=0; m[7]=0;
        m[8]=0; m[9]=0; m[10]=1; m[11]=0;
        m[12]=v.x; m[13]=v.y; m[14]=v.z; m[15]=1;
    }
    ViewMatrix operator*(const ViewMatrix &o) const; // 0x47dea0
    ViewMatrix &TurnX(float angle);                  // 0x47dcd0
    ViewMatrix &TurnY(float angle);                  // 0x47dd20
    ViewMatrix &SetTranslation(const ViewVector &v); // 0x47e330
};
struct ViewSettings {
    ViewVector offset;
    int autoLeashMode, lookAtReturnMode;
    float leashLength, outLeashSpeed, povLeashSpeed;
    float cameraOutPitch, cameraPovPitch, cameraOutYaw, cameraPovYaw;
    float lookAtAttenPitch, lookAtAttenYaw, lookAtPitchRange, lookAtYawRange;
    float flyPitchRange, flyYawRange, maxJumpHeight;
    float maxFlyPitchAdjust, maxFlyYawAdjust, maxJumpPitchAdjust;
    float lookAtTurnValue, returnToBaseTime;
};
struct ViewBody {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual ViewMatrix *Matrix();                    // +0x0c
    virtual void v4(); virtual float Speed();        // +0x14
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual int OnGround();                         // +0x3c
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19(); virtual void v20();
    virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24(); virtual void v25();
    virtual float Pitch(); virtual float Roll();     // +0x68, +0x6c
};
struct ViewSoul {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24();
    virtual int State();                            // +0x64
    char pad[0x7c-4]; float jumpStartY;
};
struct ViewApplication { void LookDelta(float *pitch, float *yaw); }; // 0x43bdc0
ViewApplication *GetApplication();                   // 0x4206b0
struct ViewCameraSoul {
    int vtable, inputVtable;
    void *camera;
    ViewBody *target;
    ViewSoul *targetSoul;
    int enabled;
    ViewVector povOffset;
    int autoLeashMode, lookAtReturnMode;
    float leashLength, leashScale, leashBlend, lookPitch, lookYaw;
    char pad40[0x148-0x40];
    int lookMode;
    ViewMatrix ComputeView(const ViewSettings *s, ViewMatrix targetMatrix);
};
inline int ViewBits(float f) { return *(int *)&f; }

ViewMatrix ViewCameraSoul::ComputeView(const ViewSettings *s, ViewMatrix targetMatrix)
{
    ViewMatrix shift, pitchTurn, unused, yawTurn, placed;
    ViewVector backOffset;
    float speed = target->Speed();
    ViewMatrix whole;
    ViewVector wholePosition;
    float lookPitchAtten = 0.0f, lookYawAtten = 0.0f;
    float jumpFraction; // Original leaves this unset when MaxJumpHeight is nonpositive.
    shift.Shift(povOffset);
    placed = shift * targetMatrix;
    if (!target->OnGround() || !autoLeashMode || targetSoul->State() == 19 ||
        targetSoul->State() == 8 || targetSoul->State() == 13 || targetSoul->State() == 0)
        leashBlend = 1.0f;
    else if (autoLeashMode) {
        if (speed < s->outLeashSpeed) leashBlend = 1.0f;
        else if (speed < s->povLeashSpeed)
            leashBlend = ((s->povLeashSpeed-speed)/(s->povLeashSpeed-s->outLeashSpeed))*leashScale;
        else leashBlend = 0.0f;
    }
    float leash;
    if (autoLeashMode) leash = leashLength * leashBlend;
    else leash = leashLength * leashScale;
    backOffset.x=0; backOffset.y=0; backOffset.z=-leash;
    shift.Shift(backOffset);
    if (lookAtReturnMode) { lookPitch *= leashBlend; lookYaw *= leashBlend; }
    ViewMatrix toPoint, fromPoint, turn;
    toPoint.Shift(povOffset * -1.0f);
    fromPoint.Shift(povOffset);
    float lookA=0.0f, lookB=0.0f;
    if (lookMode == 0) {
        GetApplication()->LookDelta(&lookA, &lookB);
        lookPitch = s->lookAtTurnValue * lookA + lookPitch;
        lookYaw = s->lookAtTurnValue * lookB + lookYaw;
        if (!(lookPitch < s->lookAtPitchRange)) lookPitch = s->lookAtPitchRange;
        else if (ViewBits(lookPitch) <= 0) lookPitch = 0.0f;
    }
    float pitch=(s->cameraPovPitch-s->cameraOutPitch)*leashBlend+lookPitch+s->cameraOutPitch;
    if (!target->OnGround()) {
        if (targetSoul->State() == 17) {
            if (ViewBits(s->flyPitchRange)>0 && !(target->Pitch()<0.0f)) {
                float t=target->Pitch()/s->flyPitchRange;
                t = t<0.0f ? 0.0f : (1.0f>t ? t : 1.0f);
                pitch=s->maxFlyPitchAdjust*t+pitch;
            }
        } else if (targetSoul->State() == 14 && ViewBits(s->maxJumpHeight)>0) {
            float t=(target->Matrix()->m[13]-targetSoul->jumpStartY)/s->maxJumpHeight;
            jumpFraction=t<0.0f ? 0.0f : (1.0f>t ? t : 1.0f);
            pitch=s->maxJumpPitchAdjust*jumpFraction+pitch;
        }
    }
    const float degrees=0.017453292f;
    float pitchRadians=pitch*degrees;
    turn.TurnX(pitchRadians);
    pitchTurn=(fromPoint*turn)*toPoint;
    float yaw=lookMode==0 ? (s->cameraPovYaw-s->cameraOutYaw)*leashBlend+lookYaw+s->cameraOutYaw : lookYaw;
    float yawRadians=yaw*degrees;
    turn.TurnY(yawRadians);
    yawTurn=(fromPoint*turn)*toPoint;
    whole=shift*(pitchTurn*(yawTurn*placed));
    wholePosition.x=whole.m[12]; wholePosition.y=whole.m[13]; wholePosition.z=whole.m[14];
    placed.SetTranslation(wholePosition);
    if (target->OnGround()) {
        lookPitchAtten=(float)(sin(pitchRadians)*s->lookAtAttenPitch);
        lookYawAtten=(float)(sin(yawRadians)*s->lookAtAttenYaw);
    }
    if (!target->OnGround()) {
        if (targetSoul->State()==17) {
            float roll=0.0f;
            if (ViewBits(s->flyYawRange)>0) {
                float t=target->Roll()/s->flyYawRange;
                roll=t < -1.0f ? -1.0f : (1.0f>t ? t : 1.0f);
            }
            lookYawAtten=s->maxFlyYawAdjust*roll+lookYawAtten;
        } else if (targetSoul->State()==14)
            lookPitchAtten=lookPitchAtten-s->maxJumpPitchAdjust*jumpFraction;
    }
    pitchTurn.TurnX((pitch-lookPitchAtten)*degrees);
    yawTurn.TurnY((yaw-lookYawAtten)*degrees);
    whole=pitchTurn*(yawTurn*placed);
    return whole;
}
