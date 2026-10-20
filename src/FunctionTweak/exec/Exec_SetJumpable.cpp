// SetJumpableFunction::Execute (0x433f20).
#include "FunctionTweak/BodyParams.h"

int SetJumpableFunction::Execute(ScriptCall *a, int extra)
{
    ParamActor *actor = (ParamActor *)((CallActor *)a)->Actor();
    BodyParams saved;
    saved = *actor->params;
    saved.i8 = (int)ARGF(0, extra);
    *actor->params = saved;
    return 1;
}
