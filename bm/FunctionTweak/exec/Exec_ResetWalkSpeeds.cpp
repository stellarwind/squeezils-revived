// ResetWalkSpeedsFunction::Execute (0x433070).
#include "FunctionTweak/BodyParams.h"

int ResetWalkSpeedsFunction::Execute(ScriptCall *a, int extra)
{
    ParamActor *actor = (ParamActor *)((CallActor *)a)->Actor();
    BodyParams saved;
    saved = *actor->params;
    BodyParams *d = actor->Defaults();
    saved.b0.v[0] = d->b0.v[0];
    saved.b0.v[1] = d->b0.v[1];
    saved.b0.v[2] = d->b0.v[2];
    saved.b1.v[2] = d->b1.v[2];
    saved.b0.v[3] = d->b0.v[3];
    saved.b0.v[4] = d->b0.v[4];
    saved.b0.v[5] = d->b0.v[5];
    *actor->params = saved;
    return 1;
}
