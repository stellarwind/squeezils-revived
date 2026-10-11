// SetFrictionFunction::Execute (0x431930): add the two operands to the creature's friction.
#include "FunctionTweak/TweakExec.h"

int SetFrictionFunction::Execute(ScriptCall *a, int extra)
{
    Physics *tuning = ((CallActor *)a)->Actor2()->Tuning();
    tuning->frictionB += ARGF(0, extra);
    tuning->frictionA += ARGF(1, extra);
    return 1;
}
