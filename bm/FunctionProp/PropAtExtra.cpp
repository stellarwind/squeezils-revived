// CreatePropAtPosition::ParseExtra (0x46fdd0).
#define V3_CTOR_OUTOFLINE
#define V3_COPYCTOR
#include "FunctionProp/PropClasses.h"

int CreatePropAtPositionFunction::ParseExtra(ArgList *list)
{
    offset = ParseVectorArgument(list, Vector3f(0, 0, 0));
    return 1;
}
