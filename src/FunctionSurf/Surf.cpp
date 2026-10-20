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
    DebugLog("%s", Name());
    first->Print();
    second->Print();
    DebugLog("\n");
}

const char *SetSurfaceTypeFunction::Name() { return "SetSurfaceType"; }
