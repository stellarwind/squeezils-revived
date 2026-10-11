// CreatePropFunction::Create (0x46f9e0).
#define V3_CTOR_OUTOFLINE
#define V3_COPYCTOR
#include "FunctionProp/Prop.h"

Function *CreatePropFunction::Create(ArgList *list)
{
    CreatePropFunction *f = Make();
    f->name = ParseStringArgument(list);
    f->position = ParseVectorArgument(list, Vector3f(0, 0, 1));
    f->direction = ParseVectorArgument(list, Vector3f(0, 1, 0));
    f->ParseExtra(list);
    return f;
}
