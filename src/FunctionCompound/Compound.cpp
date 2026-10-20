// CompoundFunction (SQZ.EXE 0x421b00-0x421c90).
#include "FunctionCompound/Compound.h"

CompoundFunction::~CompoundFunction()
{
    for (int i = 0; i < items.count; i++) {
        Function *f = items.Get(i);
        if (f)
            delete f;
    }
}

int CompoundFunction::Execute(ScriptCall *args, int extra)
{
    for (int i = 0; i < items.count; i++) {
        if (!items.Get(i)->Execute(args, extra))
            return 0;
    }
    return 1;
}

Function *CompoundFunction::Create(ArgList *list)
{
    return 0;
}

const char *CompoundFunction::Name() { return "Compound"; }
