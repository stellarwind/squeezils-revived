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

void PlayAmbientLoop(String name, int a, float b, float c);          // 0x44a5a0 (cdecl)
struct AmbVec { float x, y, z; AmbVec() {} ~AmbVec(); };              // Vector3f, dtor 0x401660

int StartAmbientLoopFunction::Execute(ScriptCall *args, int extra)   // 0x43a4d0
{
    AmbVec unused;                      // leftover local in the original
    PlayAmbientLoop(sound, a->Get(extra), b->Get(extra), c->Get(extra));
    return 1;
}
