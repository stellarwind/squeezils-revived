// PartCycle::Sample (SQZ.EXE 0x419960): fill a pose at `time`. Between two samples the position, direction and up are
// blended linearly (the two axes renormalised) and so are the morph weights; outside the track the nearest sample is copied.
// Returns the sample index (the hint after FindSegment).
#include "PartCycle/Execution.h"

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

int PartCycleN::Sample(PartPose *pose, float time, int hint)
{
    int weightsPerSample = -1;
    if (morph.count > 0)
        weightsPerSample = morph.Get(0)->count;
    int i;
    int seg = hint;
    if (((PartCycleF *)this)->FindSegment(time, &seg) != -1) {
        MorphCycleF *c = ((PartCycleF *)this)->cycle;
        float t0 = c->TimeAt(seg);
        float f = (time - t0) / (c->TimeAt(seg + 1) - t0);
        pose->position = positions.count > 0
            ? positions.Get(seg) * (1.0f - f) + positions.Get(seg + 1) * f
            : Vector3f(0.0f, 0.0f, 0.0f);
        pose->direction = orientations.count > 0
            ? orientations.Get(seg).direction * (1.0f - f) + orientations.Get(seg + 1).direction * f
            : Vector3f(0.0f, 0.0f, 1.0f);
        pose->direction.Normalize();
        pose->up = orientations.count > 0
            ? orientations.Get(seg).up * (1.0f - f) + orientations.Get(seg + 1).up * f
            : Vector3f(0.0f, 1.0f, 0.0f);
        pose->up.Normalize();
        if (morph.count > 0) {
            for (i = 0; i < pose->weights.count; i++) {
                float v;
                if (weightsPerSample < 1 || i >= weightsPerSample)
                    v = 0.0f;
                else
                    v = morph.Get(seg)->Get(i) * (1.0f - f) + morph.Get(seg + 1)->Get(i) * f;
                pose->weights.Replace(i, v);
            }
        }
        } else {
        pose->position = positions.count > 0 ? positions.Get(seg) : Vector3f(0.0f, 0.0f, 0.0f);
        pose->direction = orientations.count > 0 ? orientations.Get(seg).direction : Vector3f(0.0f, 0.0f, 1.0f);
        pose->up = orientations.count > 0 ? orientations.Get(seg).up : Vector3f(0.0f, 1.0f, 0.0f);
        if (morph.count > 0) {
            for (i = 0; i < pose->weights.count; i++) {
                WeightList *w = morph.Get(seg);
                float v = i < weightsPerSample ? w->Get(i) : 0.0f;
                pose->weights.Replace(i, v);
            }
        }
    }
    return seg;
}
