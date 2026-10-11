// SetHopFunction::Execute (0x434890).
#include "FunctionTweak/BodyParams.h"

int SetHopFunction::Execute(ScriptCall *a, int extra)
{
    ParamActor *actor = (ParamActor *)((CallActor *)a)->Actor();
    BodyParams saved;
    saved = *actor->params;
    saved.d1.v[0] += ARGF(0, extra);
    saved.d1.v[1] += ARGF(1, extra);
    saved.d1.v[2] += ARGF(2, extra);
    *actor->params = saved;
    return 1;
}
