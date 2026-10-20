// RegionCameraSoul::Update (SQZ.EXE 0x409f70, vtable slot 5): restart the blend when the target's up vector turned,
// bring mouse-look back to rest, then aim the camera (blending from the old matrix while a blend runs).
#include "CameraUpdate.h"

struct RegionCameraSoulU : CameraSoulU { void Update(); };

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
