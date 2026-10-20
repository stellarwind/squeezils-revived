// AddSpitNutsFunction (vtable 0x49db14) and ClearSenseTargetsFunction (0x49db2c).
#pragma once
#include "FunctionArgs/FunctionArgs.h"

struct SpitCall {                        // ScriptCall seen through its accessors
    struct Squeezil *Actor();            // 0x420c20
    int Busy();                          // 0x420c60
};
struct Squeezil { void AddSpitNuts(int n); };   // 0x456b40

class AddSpitNutsFunction : public Function {
public:
    AddSpitNutsFunction() { n = 0; }
    ~AddSpitNutsFunction() { delete n; }
    virtual int Execute(ScriptCall *args, int extra);    // 0x423ca0
    virtual Function *Create(ArgList *list);             // 0x423b00
    virtual void Print();                                // 0x423c70
    virtual const char *Name();                          // 0x423bc0
    IntFunctionArgument *n;                              // 0x04
};
