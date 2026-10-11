// IfOnSurfaceType and its two subclasses: the small slots.
#include "FunctionIf/IfSurface.h"

const char *IfOnSurfaceTypeFunction::Name() { return "IfOnSurfaceType"; }

int IfOnSurfaceTypeFunction::Parse(ArgList *list)
{
    type = ParseStringArgument(list);
    return 1;
}

void IfOnSurfaceTypeFunction::Dump()
{
    type->Print();
}

IfFunction *IfOnSurfaceTypeFunction::Make() { return new IfOnSurfaceTypeFunction; }

const char *IfOnFaceSetFunction::Name() { return "IfOnFaceSet"; }
IfFunction *IfOnFaceSetFunction::Make() { return new IfOnFaceSetFunction; }

const char *IfOnOrAboveSurfaceTypeFunction::Name() { return "IfOnOrAboveSurfaceType"; }
IfFunction *IfOnOrAboveSurfaceTypeFunction::Make() { return new IfOnOrAboveSurfaceTypeFunction; }

IfOnSurfaceTypeFunction::~IfOnSurfaceTypeFunction()
{
    if (type)
        delete type;
}
