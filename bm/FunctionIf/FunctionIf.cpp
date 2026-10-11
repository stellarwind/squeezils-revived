// IfFunction base slots (SQZ.EXE 0x443720-0x443800).
#include "FunctionIf/FunctionIf.h"

int IfFunction::Execute(ScriptCall *args, int extra)
{
    if (Test(args, extra)) {
        ExecContext *ctx = new ExecContext(args);
        int r = child->Execute((ScriptCall *)ctx, extra);
        delete ctx;
        return r;
    }
    return 1;
}

Function *IfFunction::Create(ArgList *list)
{
    IfFunction *f = Make();
    if (f->Parse(list)) {
        f->child = ParseFunction(list);
        return f->child ? f : 0;
    }
    return 0;
}

void IfFunction::Print()
{
    DebugLog("%s", Name());
    Dump();
    child->Print();
}
