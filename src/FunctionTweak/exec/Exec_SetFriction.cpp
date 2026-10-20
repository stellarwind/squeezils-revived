// SetFrictionFunction::Execute (0x431930): add the two operands to the creature's friction.
// Three spare declarations preserve the original register allocation (padscan.sh).
void PadDecl1(); void PadDecl2(); void PadDecl3();
#include "FunctionTweak/TweakExec.h"

int SetFrictionFunction::Execute(ScriptCall *a, int extra)
{
    Physics *tuning = ((CallActor *)a)->Actor2()->Tuning();
    tuning->frictionB += ARGF(0, extra);
    tuning->frictionA += ARGF(1, extra);
    return 1;
}
