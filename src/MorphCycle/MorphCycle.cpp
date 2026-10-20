// MorphCycle getters (SQZ.EXE 0x41ba00-0x41ba40).
#include "MorphCycle/MorphCycle.h"

float MorphCycle::Duration()   { return duration; }
float MorphCycle::Period() { return period; }
float MorphCycle::Speed()      { return speed; }
int   MorphCycle::SampleCount() { return sampleCount; }
int   MorphCycle::NoLoop()     { return noLoop; }
