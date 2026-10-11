// ExecuteWithPropFunction (SQZ.EXE 0x4449e0-0x444cb0).
#include "FunctionWithProp/WithProp.h"

Function *ExecuteWithPropFunction::Create(ArgList *list)
{
    ExecuteWithPropFunction *f = new ExecuteWithPropFunction;
    PropArgList *l = (PropArgList *)list;
    name = l->ReadString(0);        // sic: the original fills the prototype (this), not the new object
    l->Skip(0);
    f->child = ParseFunction(list);
    return f->child ? f : 0;
}

int ExecuteWithPropFunction::Execute(ScriptCall *args, int extra)
{
    PropCall *c = (PropCall *)args;
    int level = 0;
    if (c->Level())
        level = c->Level();
    FindProp(name, level, 0);
    ExecContext *ctx = new ExecContext(args);
    int r = child->Execute((ScriptCall *)ctx, extra);
    delete ctx;
    return r;
}

void ExecuteWithPropFunction::Print()
{
    DebugLog("%s %s ", Name(), name.c_str());
    child->Print();
}

const char *ExecuteWithPropFunction::Name() { return "ExecuteWithProp"; }
