// 4 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
// AddSpitNutsFunction (SQZ.EXE 0x423b00-0x423d80).
#include "FunctionSpit/Spit.h"

Function *AddSpitNutsFunction::Create(ArgList *list)
{
    IntFunctionArgument *arg = (IntFunctionArgument *)ParseArgument(list);
    if (arg->Get(0) <= 0) {
        DebugLog("WARNING: AddSpitNuts n must be positive.\n");
        return 0;
    }
    AddSpitNutsFunction *f = new AddSpitNutsFunction;
    f->n = arg;
    return f;
}

int AddSpitNutsFunction::Execute(ScriptCall *args, int extra)
{
    SpitCall *c = (SpitCall *)args;
    Squeezil *s = c->Actor();
    if (!s) {
        DebugLog("WARNING: attempt to run AddSpitNuts on non-Squeezil %s", c->Busy());
        return 0;
    }
    s->AddSpitNuts(n->Get(0));
    return 1;
}

void AddSpitNutsFunction::Print()
{
    DebugLog("%s", Name());
    n->Print();
    DebugLog("\n");
}

const char *AddSpitNutsFunction::Name() { return "AddSpitNuts"; }
