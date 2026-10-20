// PlaceCameraSoul::Update (SQZ.EXE 0x40a820, vtable slot 5): while a camera path plays (enabled == 0) follow the path's
// "part" placement (in world space, at the player's position, or relative to the player's matrix), blending in from the
// old camera matrix; otherwise the RegionCameraSoul update with the [PlaceCamera] settings, plus an on-ground check
// before a turn restarts the blend, and the player is unfrozen when the blend ends.
#include "CameraUpdate.h"

struct PlaceCameraSoulU : CameraSoulU { void Update(); };

void PlaceCameraSoulU::Update()
{
    CamMat blended;
    CamMat result;
    if (enabled) {
        if (body->IsOnGround() && !justStarted) {
            CamMatData *cur = body->GetMatrix();
            if (!(body->lastMatrix.m[4] == cur->m[4] && cur->m[5] == body->lastMatrix.m[5] && body->lastMatrix.m[6] == cur->m[6])) {
                blendTime = 0.0f;
                blendDuration = gCamInterpTime;
                fromMatrix = body->lastMatrix;
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
                returnPitchRate = -(lookPitch / gPlaceSettings.returnToBaseTime);
                returnYawRate = -(lookYaw / gPlaceSettings.returnToBaseTime);
                lookMode = 2;
            }
            if (lookMode == 2) {
                if (BITS(lookPitch) > 0) {
                    lookPitch = returnPitchRate + lookPitch;
                    if (BITS(lookPitch) <= 0)
                        lookPitch = 0.0f;
                }
                float yawSign = 1.0f;
                lookYaw = returnYawRate + lookYaw;
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
            if (!(blendTime < blendDuration)) {
                blending = 0;
                targetSoul->Unfreeze();
                result.d = Compute(&gPlaceSettings, toMatrix).d;
            } else {
                toMatrix.d = *body->GetMatrix();
                blended.d = BlendMatrix().d;
                result.d = Compute(&gPlaceSettings, blended).d;
            }
        } else {
            result.d = Compute(&gPlaceSettings, *(CamMat *)body->GetMatrix()).d;
            if (justStarted == 1)
                justStarted = 0;
        }
        view->SetMatrix(&result);
    } else {
        path->Advance((float)TheClock()->SimRate() * 0.001f, 0);
        CamMat m;
        CamMat pose = path->poses->Find("part")->Transform();
        if (pathMode == 1) {
            m.d = pose.d;
        } else if (pathMode == 2) {
            m.d = pose.d;
            m.SetTranslation(*(CamVec *)&m.d.m[12] + *(CamVec *)&body->GetMatrix()->m[12]);
        } else {
            m.d = (pose * *(CamMat *)body->GetMatrix()).d;
        }
        if (blending) {
            blendTime = (float)TheClock()->SimRate() * 0.001f + blendTime;
            if (!(blendTime < blendDuration)) {
                blending = 0;
                result = m;
            } else {
                toMatrix = m;
                result = BlendMatrix();
            }
            view->SetMatrix(&result);
        } else {
            view->SetMatrix(&m);
        }
    }
}
