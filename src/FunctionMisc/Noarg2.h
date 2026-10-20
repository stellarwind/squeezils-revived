// More no-operand commands whose Create is just `new` (hand-written; gen.pl could not size their folded slots).
#pragma once
#include "FunctionArgs/FunctionArgs.h"

class ClearSenseTargetsFunction : public Function {
public:
    ~ClearSenseTargetsFunction() {}
    virtual int Execute(ScriptCall *args, int extra);    // 0x423e20
    virtual Function *Create(ArgList *list);             // 0x423cf0
    virtual void Print();                                // 0x423e00
    virtual const char *Name();                          // 0x423d80
};

class ClearMessageFunction : public Function {
public:
    virtual int Execute(ScriptCall *args, int extra);    // 0x4406c0
    virtual Function *Create(ArgList *list);             // 0x440590
    virtual void Print();                                // 0x4406a0
    virtual const char *Name();                          // 0x440620
};

class AbortFunction : public Function {
public:
    ~AbortFunction() {}
    virtual int Execute(ScriptCall *args, int extra);    // 0x4446b0
    virtual Function *Create(ArgList *list);             // 0x444620
    virtual void Print();                                // 0x444740
    virtual const char *Name();                          // 0x4446c0
};
