// SQZ.EXE 0x41df50: advance clip time, blending and the current part poses.
#include "compat41.h"

struct AdvancePose {
    void Assign(const AdvancePose &other); // 0x41b760
    ~AdvancePose(); // 0x41b5f0
};
struct AdvanceClip {
    float Duration(); // 0x41ba00
    float Period(); // 0x41ba10
    float Speed(); // 0x41ba20
    int NoLoop(); // 0x41ba40
    int SamplePose(AdvancePose *pose, float time, int hint); // 0x41bbf0
    int BlendPose(AdvancePose *pose, float time, float amount, int hint); // 0x41bcf0
};
struct AdvancingBody {
    virtual void slot0(); virtual void slot1(); virtual void slot2(); virtual void slot3();
    virtual void ApplyPose(); // slot 4, +0x10
    char partTable[0x28];
    AdvanceClip *cycle; // +0x2c
    float time; // +0x30
    int hint, direction, loopMode; // +0x34, +0x38, +0x3c
    AdvancePose *pose, *previousPose; // +0x40, +0x44
    float blendLeft; // +0x48
    void Advance(float seconds, int skipApply);
};

void AdvancingBody::Advance(float seconds, int skipApply)
{
    seconds = (float)direction * seconds;
    seconds = seconds * (1.0f / cycle->Speed());
    if (previousPose) {
        float amount = seconds;
        if (*(unsigned *)&seconds > 0x80000000U)
            amount = -seconds;
        blendLeft -= amount;
        if (blendLeft < 0.0f) {
            delete previousPose;
            previousPose = 0;
            blendLeft = 0.0f;
            if (!skipApply) {
                hint = cycle->SamplePose(pose, time, hint);
                ApplyPose();
            }
            return;
        }
        if (!skipApply) {
            pose->Assign(*previousPose);
            hint = cycle->BlendPose(pose, time, 1.0f - blendLeft / cycle->Period(), hint);
            ApplyPose();
        }
        return;
    }
    if (time + seconds > cycle->Duration()) {
        if (loopMode == 0 || cycle->NoLoop()) {
            time = cycle->Duration();
            direction = 0;
        } else if (loopMode == -1) {
            direction = -direction;
            float back = time - seconds;
            if (back > 0.0f)
                time = back;
        } else {
            time += seconds;
            if (cycle->Duration() == 0.0f)
                time = 0.0f;
            else {
                while (cycle->Duration() < time)
                    time -= cycle->Duration();
            }
        }
    } else {
        float moved = time + seconds;
        if (moved >= 0.0f)
            time = moved;
        else if (loopMode == -1) {
            direction = -direction;
            if (time - seconds <= cycle->Duration())
                time = time - seconds;
        } else if (loopMode == 0) {
            time = 0.0f;
            direction = 0;
        } else {
            time = moved;
            if (cycle->Duration() == 0.0f)
                time = 0.0f;
            else if (time < 0.0f) {
                do {
                    time = cycle->Duration() + time;
                } while (time < 0.0f);
            }
        }
    }
    if (!skipApply) {
        hint = cycle->SamplePose(pose, time, hint);
        ApplyPose();
    }
}
