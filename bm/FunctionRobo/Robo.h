// ChangeRoboResponseFunction (vtable 0x49dafc): change how a robot answers to a stimulus.
#pragma once
#include "FunctionExec/Exec.h"

struct KindArgument {                    // BehaviorTypeFunctionArgument-like: names a robot kind
    int Resolve(int context);            // 0x4235b0
};
KindArgument *ParseKindArgument(ArgList *list);          // 0x423490 (cdecl)

class ChangeRoboResponseFunction : public Function {
public:
    ChangeRoboResponseFunction() { kind = 0; response = 0; }
    virtual int Execute(ScriptCall *args, int extra);    // 0x423a50
    virtual Function *Create(ArgList *list);             // 0x423860
    virtual void Print();                                // 0x423a10
    virtual const char *Name();                          // 0x423920
    KindArgument *kind;                                  // 0x04
    StringFunctionArgument *response;                    // 0x08
};
