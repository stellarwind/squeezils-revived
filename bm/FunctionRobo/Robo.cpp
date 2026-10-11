// ChangeRoboResponseFunction (SQZ.EXE 0x423860-0x423a10).
#include "FunctionRobo/Robo.h"

Function *ChangeRoboResponseFunction::Create(ArgList *list)
{
    KindArgument *k = ParseKindArgument(list);
    StringFunctionArgument *s = ParseStringArgument(list);
    if (k->Resolve(0) == -1)
        return 0;
    ChangeRoboResponseFunction *f = new ChangeRoboResponseFunction;
    f->kind = k;
    f->response = s;
    return f;
}

const char *ChangeRoboResponseFunction::Name() { return "ChangeRoboResponse"; }
