// DelayedExecute / RenderDelayedExecute (SQZ.EXE 0x444ca0-0x445150).
#include "FunctionExec/Delayed.h"

DelayedExecuteFunction::DelayedExecuteFunction()
{
    delay = 0;
    name = 0;
}

DelayedExecuteFunction::~DelayedExecuteFunction()
{
    if (delay)
        delete delay;
    if (name)
        delete name;
}

int DelayedExecuteFunction::Execute(ScriptCall *args, int extra)
{
    const String &s = name->Get(extra);
    ScheduleDelayed(s.c_str(), delay->Get(extra), args, extra);
    return 1;
}

void DelayedExecuteFunction::Print()
{
    DebugLog("%s", Name());
    delay->Print();
    name->Print();
    DebugLog("\n");
}

const char *DelayedExecuteFunction::Name() { return "DelayedExecute"; }

int RenderDelayedExecuteFunction::Execute(ScriptCall *args, int extra)
{
    const String &s = name->Get(extra);
    ScheduleRenderDelayed(s.c_str(), delay->Get(extra), args, extra);
    return 1;
}

const char *RenderDelayedExecuteFunction::Name() { return "RenderDelayedExecute"; }
