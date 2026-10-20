// SetSurfaceTypeFunction small slots (SQZ.EXE 0x440830-0x4409f0+).
#include "FunctionSurf/Surf.h"

SetSurfaceTypeFunction::~SetSurfaceTypeFunction()
{
    if (first)
        delete first;
    if (second)
        delete second;
}

void SetSurfaceTypeFunction::Print()
{
    DebugLogAll("%s", Name());
    first->Print();
    second->Print();
    DebugLogAll("\n");
}

const char *SetSurfaceTypeFunction::Name() { return "SetSurfaceType"; }
