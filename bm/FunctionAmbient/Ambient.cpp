// StartAmbientLoopFunction (SQZ.EXE 0x43a230-0x43a4d0).
#include "FunctionAmbient/Ambient.h"

Function *StartAmbientLoopFunction::Create(ArgList *list)
{
    StartAmbientLoopFunction *f = new StartAmbientLoopFunction;
    AmbArgList *l = (AmbArgList *)list;
    f->sound = l->ReadString(0);
    l->Skip(0);
    f->a = (IntFunctionArgument *)ParseArgument(list);
    f->b = ParseFloatArgument(list);
    f->c = ParseFloatArgument(list);
    return f;
}

void StartAmbientLoopFunction::Print()
{
    DebugLog("%s %s ", Name(), sound.c_str());
    a->Print();
    b->Print();
    c->Print();
    DebugLog("\n");
}

const char *StartAmbientLoopFunction::Name() { return "StartAmbientLoop"; }
