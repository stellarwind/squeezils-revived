// PartCycle::NewPose (SQZ.EXE 0x41a3f0): a fresh pose sized to this track, filled by Sample.
#include "PartCycle/PartCycle.h"

struct PartPose {
    Vector3f position, direction, up;
    WeightList weights;
    PartPose(const Vector3f &p, const Vector3f &d, const Vector3f &u, const WeightList *w);   // 0x4193c0
    void *operator new(unsigned size);                                                        // 0x45dd40
};
struct PartCycleN : PartCycle {
    int Sample(PartPose *pose, float time, int hint);   // 0x419960
    PartPose *NewPose(float time, int hint);
};

PartPose *PartCycleN::NewPose(float time, int hint)
{
    Vector3f zero(0.0f, 0.0f, 0.0f);
    WeightList *sizing = 0;
    bool owns = false;
    if (weightCount != -1) {
        owns = true;
        sizing = new WeightList;
        for (int i = 0; i < weightCount; i++)
            sizing->Insert(0.0f, -1);
    } else {
        owns = false;
        if (morph.count > 0)
            weightCount = morph.Get(0)->count;
        else
            weightCount = 0;
        if (weightCount > 0)
            sizing = morph.Get(0);
        else
            sizing = 0;
    }
    PartPose *pose = new PartPose(zero, zero, zero, sizing);
    if (owns && sizing)
        delete sizing;
    Sample(pose, time, hint);
    return pose;
}
