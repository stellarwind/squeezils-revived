// MorphCycle::TimeAt (SQZ.EXE 0x41ba50): times->Get(firstSample + sample), a virtual call (Array slot 0).
#include "Container/ContainerSet.h"
#include "MorphCycle/MorphCycle.h"

float MorphCycle::TimeAt(int sample)
{
    return ((PrimitiveArray<float, 2> *)times)->Get(firstSample + sample);
}
