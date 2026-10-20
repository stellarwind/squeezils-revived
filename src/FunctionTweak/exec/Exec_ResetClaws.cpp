// ResetClawsFunction::Execute (0x4321a0): put the creature's claw parameters back to the defaults.
#include "FunctionTweak/BodyParams.h"

int ResetClawsFunction::Execute(ScriptCall *a, int extra)
{
    ParamActor *actor = (ParamActor *)((CallActor *)a)->Actor();
    BodyParams saved;
    saved = *actor->params;
    BodyParams *d = actor->Defaults();
    saved.f0 = d->f0;
    saved.f1 = d->f1;
    *actor->params = saved;
    return 1;
}
