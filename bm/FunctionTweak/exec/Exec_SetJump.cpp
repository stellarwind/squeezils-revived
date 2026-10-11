// SetJumpFunction::Execute (0x433550).
#include "FunctionTweak/BodyParams.h"

int SetJumpFunction::Execute(ScriptCall *a, int extra)
{
    ParamActor *actor = (ParamActor *)((CallActor *)a)->Actor();
    BodyParams saved;
    saved = *actor->params;
    saved.d0.v[0] += ARGF(0, extra);
    saved.d0.v[1] += ARGF(1, extra);
    saved.d0.v[2] += ARGF(2, extra);
    *actor->params = saved;
    return 1;
}
