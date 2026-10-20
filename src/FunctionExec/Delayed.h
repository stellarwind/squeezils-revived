// DelayedExecuteFunction (vtable 0x49e914) and RenderDelayedExecuteFunction (0x49e92c): run a script function later.
#pragma once
#include "FunctionExec/Exec.h"
#include "FunctionMisc/MiscClasses.h"

void ScheduleDelayed(const char *name, float delay, void *args, int extra);         // 0x422e00 (cdecl)
void ScheduleRenderDelayed(const char *name, float delay, void *args, int extra);   // 0x422ed0 (cdecl)

class DelayedExecuteFunction : public Function {
public:
    DelayedExecuteFunction();                            // 0x444ca0
    ~DelayedExecuteFunction();                           // 0x444d30
    virtual int Execute(ScriptCall *args, int extra);    // 0x444ed0
    virtual Function *Create(ArgList *list);             // 0x444df0
    virtual void Print();                                // 0x444e90
    virtual const char *Name();                          // 0x444f70
    FloatFunctionArgument *delay;                        // 0x04
    StringFunctionArgument *name;                        // 0x08
};

class RenderDelayedExecuteFunction : public DelayedExecuteFunction {
public:
    ~RenderDelayedExecuteFunction() {}                   // 0x445030 sdd
    virtual int Execute(ScriptCall *args, int extra);    // 0x4450a0
    virtual Function *Create(ArgList *list);             // 0x444f80
    virtual const char *Name();                          // 0x445140
};
