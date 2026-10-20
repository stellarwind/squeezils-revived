// HidePropFunction::Execute (0x4702c0).
#include "FunctionStr/PropExec.h"

int HidePropFunction::Execute(ScriptCall *args, int extra)
{
    PropV *p = FindPropV(text, ((PropLevel *)args)->Level(), 1);
    if (p)
        p->Hide();
    else
        DebugLogAll("WARNING: HideProp couldn't find prop %s", text.c_str());
    return 1;
}
