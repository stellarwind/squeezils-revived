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
    DebugLog("%s", Name());
    a->Print();
    b->Print();
    c->Print();
    DebugLog("%s", path.c_str());
    DebugLog("\n");
}

const char *AnimateCameraPathFunction::Name() { return "AnimateCameraPath"; }
