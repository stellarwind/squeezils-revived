// 29 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
void PadDecl15();
void PadDecl16();
void PadDecl17();
void PadDecl18();
void PadDecl19();
void PadDecl20();
void PadDecl21();
void PadDecl22();
void PadDecl23();
void PadDecl24();
void PadDecl25();
void PadDecl26();
void PadDecl27();
void PadDecl28();
void PadDecl29();
// IfVariableFunction base slots (SQZ.EXE 0x443fb0-0x4440b0).
#include "FunctionIf/IfVar.h"

int IfVariableFunction::Parse(ArgList *list)
{
    name = list->ReadString(0);
    list->Skip(0);
    return 1;
}

void IfVariableFunction::Dump()
{
    DebugLogAll("%s", name.c_str());
}

int IfVariableFunction::Test(ScriptCall *args, int extra)
{
    VariableTable *t = &g_variables;
    if (!t->Contains(&name))
        return 0;
    return Check(t->Get(&name), args, extra);
}

int IfVariableValueFunction::Parse(ArgList *list)
{
    IfVariableFunction::Parse(list);
    value = (IntFunctionArgument *)ParseArgument(list);
    return 1;
}

void IfVariableValueFunction::Dump()
{
    IfVariableFunction::Dump();
    value->Print();
}

int IfVariableValueFunction::Check(int v, ScriptCall *args, int extra)
{
    return Compare(v, value->Get(extra), args);
}

const char *IfVariableEqualFunction::Name() { return "IfEqual"; }
IfFunction *IfVariableEqualFunction::Make() { return new IfVariableEqualFunction; }
int IfVariableEqualFunction::Compare(int v, int ref, ScriptCall *args) { return ref == v; }

const char *IfVariableLesserFunction::Name() { return "IfLesser"; }
IfFunction *IfVariableLesserFunction::Make() { return new IfVariableLesserFunction; }
int IfVariableLesserFunction::Compare(int v, int ref, ScriptCall *args) { return ref > v; }

const char *IfVariableGreaterFunction::Name() { return "IfGreater"; }
IfFunction *IfVariableGreaterFunction::Make() { return new IfVariableGreaterFunction; }
int IfVariableGreaterFunction::Compare(int v, int ref, ScriptCall *args) { return ref < v; }
