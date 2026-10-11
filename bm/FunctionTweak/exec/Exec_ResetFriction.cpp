// 4 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
// ResetFrictionFunction::Execute (0x431b50): put the creature's friction back to its default.
#include "FunctionTweak/TweakExec.h"

int ResetFrictionFunction::Execute(ScriptCall *a, int extra)
{
    TweakActor *actor = ((CallActor *)a)->Actor();
    Physics *tuning = actor->Tuning();
    Physics *defaults = actor->Defaults();
    tuning->frictionB = defaults->frictionB;
    tuning->frictionA = defaults->frictionA;
    return 1;
}
