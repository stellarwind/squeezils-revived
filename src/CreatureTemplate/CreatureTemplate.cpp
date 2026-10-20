// CreatureTemplate ctor (0x4483d0) and dtor (0x4484a0).
#include "CreatureTemplate/CreatureTemplate.h"

CreatureTemplate::CreatureTemplate()
{
    for (int i = 0; i < 20; i++) {
        names.Insert(String(""), -1);
    }
    float *sr = senseRadius;
    sr[2] = sr[1] = sr[0] = 0.0f;
}

CreatureTemplate::~CreatureTemplate()
{
    PrimitiveArray<String, 3> *list = &names;
    for (int i = 19; i >= 0; i--)
        list->Remove(i);
}
