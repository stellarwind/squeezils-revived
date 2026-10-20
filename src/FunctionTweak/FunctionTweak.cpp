// TweakFloatFunction base (SQZ.EXE 0x430530-0x4306e1) and ResetClawsFunction's small slots.
#include "FunctionTweak/FunctionTweak.h"

int TweakFloatFunction::ArgCount()
{
    return 0;
}

void TweakFloatFunction::Print()
{
    DebugLogAll("%s", Name());
    for (int i = 0; i < ArgCount(); i++)
        args.Get(i)->Print();
    DebugLogAll("\n");
}

TweakFloatFunction::~TweakFloatFunction()
{
    for (int i = args.count - 1; i >= 0; --i) {
        delete args.Get(i);
        args.Remove(i);
    }
}
