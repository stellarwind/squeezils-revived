// SetCyclePropFunction base slots (SQZ.EXE 0x470360-0x470440).
#include "FunctionCycle/Cycle.h"

int SetCyclePropFunction::Execute(ScriptCall *args, int extra)
{
    void *found = FindProp(name, ((CallLevel *)args)->Level(), 0);
    if (found) {
        CycleProp *prop = (CycleProp *)DynamicCast(found, 0, kPropType, kCyclePropType, 0);
        return Apply(prop, args, extra);
    }
    DebugLog("WARNING: SetCyclePropFunction couldn't find prop %s", name.c_str());
    return 1;
}

Function *SetCyclePropFunction::Create(ArgList *list)
{
    SetCyclePropFunction *f = Make();
    ArgList2 *l = (ArgList2 *)list;
    f->name = l->ReadString(0);
    l->Skip(0);
    f->value = ParseFloatArgument(list);
    return f;
}

void SetCyclePropFunction::Print()
{
    const char *s = name.c_str();
    float v = value->Get(0);
    DebugLog("%s %s %0.02f\n", Name(), s, (double)v);
}
