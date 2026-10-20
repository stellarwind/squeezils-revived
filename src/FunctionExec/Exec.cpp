// ExecuteFunction (SQZ.EXE 0x4447d0-0x444a00).
#include "FunctionExec/Exec.h"

ExecuteFunction::ExecuteFunction()
{
    name = 0;
}

const char *ExecuteFunction::Name() { return "Execute"; }

ExecuteFunction::~ExecuteFunction()
{
    delete name;
}

Function *ExecuteFunction::Create(ArgList *list)
{
    ExecuteFunction *f = new ExecuteFunction;
    f->name = ParseStringArgument(list);
    return f;
}

void ExecuteFunction::Print()
{
    DebugLog("%s", Name());
    name->Print();
    DebugLog("\n");
}

int ExecuteFunction::Execute(ScriptCall *args, int extra)
{
    const String &s = name->Get(extra);
    PostFunction(s.c_str(), args, extra, 0);
    return 1;
}
