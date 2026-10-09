// MorphCycle ctor (0x41b7b0): empty clip with its times and parts lists made.
#include "Container/ContainerTable.h"
#include "PartCycle/PartCycle.h"
#include "String/String.h"
#include "MorphCycle/MorphCycle.h"

typedef PrimitiveArray<float, 2> WeightList;
typedef Table<String, PartCycle *, Array<PartCycle *>, 0> PartCycleTable;

MorphCycle::MorphCycle(int noLoop, float speed, float period)
{
    this->noLoop = noLoop;
    times = new WeightList;
    parts = new PartCycleTable;
    this->speed = speed;
    this->period = period;
    sharesData = 0;
}
