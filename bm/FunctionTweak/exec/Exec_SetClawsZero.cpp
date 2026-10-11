// SetClawsZeroFunction::Execute (0x432660).
#include "FunctionTweak/BodyParams.h"

int SetClawsZeroFunction::Execute(ScriptCall *a, int extra)
{
    ParamActor *actor = (ParamActor *)((CallActor *)a)->Actor();
    BodyParams saved;
    saved = *actor->params;
    saved.f0 = 0;
    saved.f1 = 0;
    *actor->params = saved;
    return 1;
}
