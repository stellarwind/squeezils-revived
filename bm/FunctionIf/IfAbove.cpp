// IfOnOrAboveSurfaceTypeFunction::Test (0x443d90): the same test with the "or above" flag set.
#include "FunctionIf/IfSurface.h"

int IfOnOrAboveSurfaceTypeFunction::Test(ScriptCall *args, int e)
{
    extra = 1;
    int r = IfOnSurfaceTypeFunction::Test(args, e);
    extra = 0;
    return r;
}
