// RegionCameraSoul::Update (SQZ.EXE 0x409f70, vtable slot 5): restart the blend when the target's up vector turned,
// bring mouse-look back to rest, then aim the camera (blending from the old matrix while a blend runs).
#include "compat41.h"
#include <math.h>

struct CamMatData {                                                // the plain 16 floats (copied inline, element by element)
    float m[16];
    CamMatData &operator=(const CamMatData &o)
    {
        m[0] = o.m[0]; m[1] = o.m[1]; m[2] = o.m[2]; m[3] = o.m[3];
        m[4] = o.m[4]; m[5] = o.m[5]; m[6] = o.m[6]; m[7] = o.m[7];
        m[8] = o.m[8]; m[9] = o.m[9]; m[10] = o.m[10]; m[11] = o.m[11];
        m[12] = o.m[12]; m[13] = o.m[13]; m[14] = o.m[14]; m[15] = o.m[15];
        return *this;
    }
};
struct CamMat {
    CamMatData d;
    CamMat() {}
    CamMat(const CamMat &o);                                       // 0x4099e0
    ~CamMat();                                                     // 0x402310
    CamMat &operator=(const CamMat &o);                            // 0x402340
};
// SIZE 0x60
struct CameraSettings {
    float povOffsetX, povOffsetY, povOffsetZ;                      // 0x00
    int autoLeashMode, lookAtReturnMode;                           // 0x0c
    float leashLength;                                             // 0x14
    char pad18[0x5c - 0x18];
    float returnToBaseTime;                                        // 0x5c
};
extern CameraSettings gRegionSettings;                             // 0x4c64e8
extern float gCamInterpTime;                                       // 0x4c6480 [Camera] CamInterpTime

struct GameClock { unsigned SimRate(); };                          // 0x41ec00: milliseconds this frame
GameClock *TheClock();                                             // 0x4206d0

struct CamBodyC {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual CamMatData *GetMatrix();                               // vtable+0x0c
    char pad4[0xc0 - 4];
    CamMatData lastMatrix;                                         // 0xc0 the matrix of the previous frame
};
struct CamViewC {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
    virtual void SetMatrix(CamMat *m);                             // vtable+0x24
};

// SIZE 0x168
struct RegionCameraSoulU {
    void *vtbl;                                                    // 0x00
    void *inputVtbl;                                               // 0x04
    CamViewC *view;                                                // 0x08
    CamBodyC *body;                                                // 0x0c
    int pad10[10];
    float lookPitch;                                               // 0x38
    float lookYaw;                                                 // 0x3c
    float returnPitchRate;                                         // 0x40
    float returnYawRate;                                           // 0x44
    CamMatData unused48;                                           // 0x48
    CamMatData fromMatrix;                                         // 0x88
    CamMatData unusedC8;                                           // 0xc8
    CamMat toMatrix;                                               // 0x108
    int lookMode;                                                  // 0x148 0 free, 1 just released, 2 returning
    float blendTime;                                               // 0x14c
    float blendDuration;                                           // 0x150
    int blending;                                                  // 0x154
    int blendPosition;                                             // 0x158
    int justStarted;                                               // 0x15c
    int pad160[2];
    CamMat Compute(CameraSettings *s, CamMat m);                   // 0x408850
    CamMat BlendMatrix();                                          // 0x409620
    void Update();
};

#define BITS(f) (*(int *)&(f))

void RegionCameraSoulU::Update()
{
    CamMat blended;
    CamMat result;
    if (!justStarted) {
        CamMatData *m = body->GetMatrix();
        CamMatData *last = &body->lastMatrix;
        if (!(last->m[4] == m->m[4] && m->m[5] == last->m[5] && last->m[6] == m->m[6])) {
            blendTime = 0.0f;
            blendDuration = gCamInterpTime;
            fromMatrix = *last;
            blending = 1;
            blendPosition = 0;
        }
    }

    if (lookMode > 0) {
        if (lookMode == 1) {
            lookPitch = (float)((int)lookPitch % 360);
            lookYaw = (float)((int)lookYaw % 360);
            if (fabs(lookYaw) > 180.0) {
                float sign = 1.0f;
                if ((unsigned)BITS(lookYaw) > 0x80000000)
                    sign = -1.0f;
                lookYaw = lookYaw - sign * 360.0f;
            }
            returnPitchRate = -(lookPitch / gRegionSettings.returnToBaseTime);
            returnYawRate = -(lookYaw / gRegionSettings.returnToBaseTime);
            lookMode = 2;
        }
        if (lookMode == 2) {
            if (BITS(lookPitch) > 0) {
                lookPitch = returnPitchRate + lookPitch;
                if (BITS(lookPitch) <= 0)
                    lookPitch = 0.0f;
            }
            float yawSign = 1.0f;
            lookYaw = lookYaw + returnYawRate;
            if ((unsigned)BITS(lookYaw) > 0x80000000)
                yawSign = -1.0f;
            float rateSign = 1.0f;
            if ((unsigned)BITS(returnYawRate) > 0x80000000)
                rateSign = -1.0f;
            if (yawSign == rateSign)
                lookYaw = 0.0f;
            if (!(BITS(lookPitch) & 0x7fffffff) && !(BITS(lookYaw) & 0x7fffffff))
                lookMode = 0;
        }
    }

    if (blending) {
        blendTime = (float)TheClock()->SimRate() * 0.001f + blendTime;
        if (!(blendTime < gCamInterpTime)) {
            blending = 0;
            result.d = Compute(&gRegionSettings, toMatrix).d;
        } else {
            toMatrix.d = *body->GetMatrix();
            blended.d = BlendMatrix().d;
            result.d = Compute(&gRegionSettings, blended).d;
        }
    } else {
        result = Compute(&gRegionSettings, *(CamMat *)body->GetMatrix());
        if (justStarted == 1)
            justStarted = 0;
    }
    view->SetMatrix(&result);
}
