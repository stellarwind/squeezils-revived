// PartCycle::Blend (SQZ.EXE 0x41a5d0): like Sample, but mixes the sampled pose into the one already there:
// pose = pose * (1 - weight) + sample * weight. Only the nearest-sample case renormalises the two axes;
// between samples they are left as blended. Outside the track a morph weight the
// track does not have is set to 0 rather than blended towards 0. Returns the sample index (the hint after FindSegment).
#include "PartCycle/PartCycle.h"

struct PartPose {
    Vector3f   position;            // 0x00
    Vector3f   direction;           // 0x0c
    Vector3f   up;                  // 0x18
    WeightList weights;             // 0x24
};
struct MorphCycleF {
    float TimeAt(int sample);       // 0x41ba50
};
struct PartCycleF {
    char pad[0x34];
    MorphCycleF *cycle;             // 0x34
    int FindSegment(float time, int *hint);   // 0x41b2a0
};
struct PartCycleB : PartCycle {
    int Blend(PartPose *pose, float time, float weight, int hint);
};

int PartCycleB::Blend(PartPose *pose, float time, float weight, int hint)
{
    int weightsPerSample = -1;
    if (morph.count > 0)
        weightsPerSample = morph.Get(0)->count;
    int seg = hint;
    int i;
    if (((PartCycleF *)this)->FindSegment(time, &seg) == -1) {
        pose->position = pose->position * (1.0f - weight) + (positions.count > 0 ? positions.Get(seg) : Vector3f(0.0f, 0.0f, 0.0f)) * weight;
        pose->direction = pose->direction * (1.0f - weight) + (orientations.count > 0 ? orientations.Get(seg).direction : Vector3f(0.0f, 0.0f, 1.0f)) * weight;
        pose->direction.Normalize();
        pose->up = pose->up * (1.0f - weight) + (orientations.count > 0 ? orientations.Get(seg).up : Vector3f(0.0f, 1.0f, 0.0f)) * weight;
        pose->up.Normalize();
        if (morph.count > 0) {
            for (i = 0; i < pose->weights.count; i++) {
                float v = i < weightsPerSample ? morph.Get(seg)->Get(i) * weight + pose->weights.Get(i) * (1.0f - weight) : 0.0f;
                pose->weights.Replace(i, v);
            }
        }
    } else {
        MorphCycleF *c = ((PartCycleF *)this)->cycle;
        float t0 = c->TimeAt(seg);
        float f = (time - t0) / (c->TimeAt(seg + 1) - t0);
        pose->position = pose->position * (1.0f - weight) + (positions.count > 0
            ? positions.Get(seg) * (1.0f - f) + positions.Get(seg + 1) * f
            : Vector3f(0.0f, 0.0f, 0.0f)) * weight;
        pose->direction = pose->direction * (1.0f - weight) + (orientations.count > 0
            ? orientations.Get(seg).direction * (1.0f - f) + orientations.Get(seg + 1).direction * f
            : Vector3f(0.0f, 0.0f, 1.0f)) * weight;
        pose->up = pose->up * (1.0f - weight) + (orientations.count > 0
            ? orientations.Get(seg).up * (1.0f - f) + orientations.Get(seg + 1).up * f
            : Vector3f(0.0f, 1.0f, 0.0f)) * weight;
        if (morph.count > 0) {
            for (i = 0; i < pose->weights.count; i++) {
                float v;
                if (weightsPerSample > 0 && i < weightsPerSample)
                    v = morph.Get(seg)->Get(i) * (1.0f - f) + morph.Get(seg + 1)->Get(i) * f;
                else
                    v = 0.0f;
                pose->weights.Replace(i, pose->weights.Get(i) * (1.0f - weight) + weight * v);
            }
        }
    }
    return seg;
}
