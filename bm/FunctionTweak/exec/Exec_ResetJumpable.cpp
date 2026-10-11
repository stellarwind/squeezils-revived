// ResetJumpableFunction::Execute (0x4343e0).
#include "FunctionTweak/BodyParams.h"

int ResetJumpableFunction::Execute(ScriptCall *a, int extra)
{
    ParamActor *actor = (ParamActor *)((CallActor *)a)->Actor();
    BodyParams saved;
    saved = *actor->params;
    BodyParams *d = actor->Defaults();
    saved.i8 = d->i8;
    *actor->params = saved;
    return 1;
}
