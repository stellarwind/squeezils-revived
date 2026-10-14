// MorphCycle copy ctor (0x41b8b0): a sub-clip over frames first..last of another clip, sharing its times and tracks.
#include "MorphCycle/MorphCycle.h"

MorphCycle::MorphCycle(const MorphCycle &whole, int first, int last, int noLoop, float speed, float period)
{
    this->noLoop = noLoop;
    this->period = (period == -1.0f) ? ((MorphCycle &)whole).Period() : period;
    this->startTime = (float)first * this->period;
    this->sampleCount = last - first + 1;
    this->firstSample = first;
    this->speed = speed;
    this->duration = (float)(last - first) * this->period;
    this->times = whole.times;
    this->sharesData = 1;
    this->parts = whole.parts;
}

