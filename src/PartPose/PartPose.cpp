// PartPose ctor (0x4193c0) and Assign (0x419530).
#include "PartPose/PartPose.h"

PartPose::PartPose(const Vector3f &p, const Vector3f &d, const Vector3f &u, const WeightList *w)
{
    position = p;
    direction = d;
    up = u;
    if (w != 0) {
        for (int i = 0; i < w->count; i++)
            weights.Insert(((WeightList *)w)->Get(i), -1);
    }
}

void PartPose::Assign(const PartPose &o)
{
    position = o.position;
    direction = o.direction;
    up = o.up;
    for (int i = 0; i < o.weights.count; i++)
        weights.Replace(i, ((WeightList &)o.weights).Get(i));
}
