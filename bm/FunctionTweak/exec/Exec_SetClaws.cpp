// SetClawsFunction::Execute (0x431cd0): add the two operands to the claw parameters.
#include "FunctionTweak/BodyParams.h"

int SetClawsFunction::Execute(ScriptCall *a, int extra)
{
    ParamActor *actor = (ParamActor *)((CallActor *)a)->Actor();
    BodyParams saved;
    saved = *actor->params;
    saved.f0 += ARGF(0, extra);
    saved.f1 += ARGF(1, extra);
    *actor->params = saved;
    return 1;
}
