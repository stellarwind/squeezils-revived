// DelayedExecuteFunction::Create (0x444df0).
#include "FunctionExec/Delayed.h"

Function *DelayedExecuteFunction::Create(ArgList *list)
{
    DelayedExecuteFunction *f = new DelayedExecuteFunction;
    f->delay = ParseFloatArgument(list);
    f->name = ParseStringArgument(list);
    return f;
}
