// SetRoboBehaviorFunction: Name, Print, Create (SQZ.EXE 0x423640-0x4237d0, 0x4237d0).
#include "FunctionRobo/Robo.h"
#include "FunctionMisc/MiscClasses.h"

struct CountedList { int pad; int count; };              // the argument list: count at +4

const char *SetRoboBehaviorFunction::Name() { return "SetRoboBehavior"; }

void SetRoboBehaviorFunction::Print()
{
    DebugLogAll("%s ", Name());
    behavior->Print();
    DebugLogAll("\n");
}

Function *SetRoboBehaviorFunction::Create(ArgList *list)
{
    BehaviorTypeArgument *b = ParseBehaviorTypeArgument(list);
    if (b->Resolve(0) == -1)
        return 0;
    SetRoboBehaviorFunction *f = new SetRoboBehaviorFunction;
    f->behavior = b;
    int n = ((CountedList *)list)->count;
    if (n > 3) {
        DebugLogAll("WARNING: too many arguments to SetRoboBehavior function.\n");
        n = 3;
    }
    for (int i = 0; i < n; i++) {
        FloatFunctionArgument *a = ParseFloatArgument(list);
        f->values[i] = a->Get(0);
        delete a;
    }
    return f;
}
