// ResetJumpFunction::Execute (0x433a50).
#include "FunctionTweak/BodyParams.h"

int ResetJumpFunction::Execute(ScriptCall *a, int extra)
{
    ParamActor *actor = (ParamActor *)((CallActor *)a)->Actor();
    BodyParams saved;
    saved = *actor->params;
    BodyParams *d = actor->Defaults();
    saved.d0.v[0] = d->d0.v[0];
    saved.d0.v[1] = d->d0.v[1];
    saved.d0.v[2] = d->d0.v[2];
    *actor->params = saved;
    return 1;
}
