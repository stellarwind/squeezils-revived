// KillSqueezilFunction (vtable 0x49e81c): slots at 0x442500 0x441600 0x442420 0x4424c0 0x4424f0.
#include "FunctionArgs/FunctionArgs.h"

struct Target {
    virtual void t0(); virtual void t1(); virtual void t2(); virtual void t3(); virtual void t4();
    virtual void t5(); virtual void t6(); virtual void t7(); virtual void t8(); virtual void t9();
    virtual void t10(); virtual void t11(); virtual void t12();
    virtual void Kill(int how);          // slot 13 (+0x34)
};

class KillSqueezilFunction : public Function {
public:
    KillSqueezilFunction() { arg = 0; }
    ~KillSqueezilFunction() { delete arg; }
    virtual int Execute(ScriptCall *args, int extra);
    virtual Function *Create(ArgList *list);
    virtual void Print();
    virtual const char *Name();
    IntFunctionArgument *arg;            // 0x04
};

int KillSqueezilFunction::Execute(ScriptCall *args, int extra)
{
    args->Actor()->Kill(arg->Get(extra));
    return 1;
}

Function *KillSqueezilFunction::Create(ArgList *list)
{
    KillSqueezilFunction *f = new KillSqueezilFunction;
    f->arg = (IntFunctionArgument *)ParseArgument(list);
    return f;
}

void KillSqueezilFunction::Print()
{
    DebugLogAll("%s %d", Name(), arg->Get(0));
}

const char *KillSqueezilFunction::Name()
{
    return "KillSqueezil";
}
