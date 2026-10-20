// AnimateCameraPathFunction (SQZ.EXE 0x44e2c0-0x44e610).
#include "FunctionCamPath/CamPath.h"

Function *AnimateCameraPathFunction::Create(ArgList *list)
{
    AnimateCameraPathFunction *f = new AnimateCameraPathFunction;
    f->a = (IntFunctionArgument *)ParseArgument(list);
    f->b = (IntFunctionArgument *)ParseArgument(list);
    f->c = ParseFloatArgument(list);
    f->path = ((CamArgList *)list)->ReadString(0);
    return f;
}

void AnimateCameraPathFunction::Print()
{
    DebugLogAll("%s", Name());
    a->Print();
    b->Print();
    c->Print();
    DebugLogAll("%s", path.c_str());
    DebugLogAll("\n");
}

const char *AnimateCameraPathFunction::Name() { return "AnimateCameraPath"; }
