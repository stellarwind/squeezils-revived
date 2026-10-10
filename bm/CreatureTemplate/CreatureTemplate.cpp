// CreatureTemplate ctor (0x4483d0) and dtor (0x4484a0).
#include "CreatureTemplate/CreatureTemplate.h"

CreatureTemplate::CreatureTemplate()
{
    PrimitiveArray<String, 3> *list = this;
    for (int i = 0; i < 20; i++) {
        String none("");
        list->Insert(none, -1);
    }
    float *sr = senseRadius;
    sr[0] = sr[1] = sr[2] = 0.0f;
}

CreatureTemplate::~CreatureTemplate()
{
    PrimitiveArray<String, 3> *list = this;
    for (int i = 19; i >= 0; i--)
        list->Remove(i);
}
