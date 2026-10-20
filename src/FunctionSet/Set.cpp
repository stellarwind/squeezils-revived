// SetTo / AddTo (SQZ.EXE 0x444130-0x444620).
#include "FunctionSet/Set.h"

Function *SetToFunction::Create(ArgList *list)
{
    SetToFunction *f = new SetToFunction;
    SetArgList *l = (SetArgList *)list;
    f->name = l->ReadString(0);
    l->Skip(0);
    f->value = (IntFunctionArgument *)ParseArgument(list);
    return f;
}

void SetToFunction::Print()
{
    const char *s = name.c_str();
    int v = value->Get(0);
    DebugLogAll("%s %s %d", Name(), s, v);
    DebugLogAll("\n");
}

const char *SetToFunction::Name() { return "SetTo"; }

Function *AddToFunction::Create(ArgList *list)
{
    AddToFunction *f = new AddToFunction;
    SetArgList *l = (SetArgList *)list;
    f->name = l->ReadString(0);
    l->Skip(0);
    f->value = (IntFunctionArgument *)ParseArgument(list);
    return f;
}

void AddToFunction::Print()
{
    const char *s = name.c_str();
    int v = value->Get(0);
    DebugLogAll("%s %s %d", Name(), s, v);
    DebugLogAll("\n");
}

const char *AddToFunction::Name() { return "AddTo"; }
