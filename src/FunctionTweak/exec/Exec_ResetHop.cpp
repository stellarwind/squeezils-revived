// ResetHopFunction::Execute (0x434d90).
#include "FunctionTweak/BodyParams.h"

int ResetHopFunction::Execute(ScriptCall *a, int extra)
{
    ParamActor *actor = (ParamActor *)((CallActor *)a)->Actor();
    BodyParams saved;
    saved = *actor->params;
    BodyParams *d = actor->Defaults();
    saved.d1.v[0] = d->d1.v[0];
    saved.d1.v[1] = d->d1.v[1];
    saved.d1.v[2] = d->d1.v[2];
    *actor->params = saved;
    return 1;
}
