// SetWalkSpeedsFunction::Execute (0x432b10): add the seven operands to the walk speed parameters.
#include "FunctionTweak/BodyParams.h"

int SetWalkSpeedsFunction::Execute(ScriptCall *a, int extra)
{
    ParamActor *actor = (ParamActor *)((CallActor *)a)->Actor();
    BodyParams saved;
    saved = *actor->params;
    saved.b0.v[0] += ARGF(0, extra);
    saved.b0.v[1] += ARGF(1, extra);
    saved.b0.v[2] += ARGF(2, extra);
    saved.b1.v[2] += ARGF(3, extra);
    saved.b0.v[3] += ARGF(4, extra);
    saved.b0.v[4] += ARGF(5, extra);
    saved.b0.v[5] += ARGF(6, extra);
    *actor->params = saved;
    return 1;
}
