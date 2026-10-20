// SetSurfaceTypeFunction (vtable 0x49e74c): override the surface type of the faces matching a name.
#pragma once
#include "FunctionExec/Exec.h"

class SetSurfaceTypeFunction : public Function {
public:
    SetSurfaceTypeFunction() { second = 0; first = 0; }
    ~SetSurfaceTypeFunction();                           // 0x440830
    virtual int Execute(ScriptCall *args, int extra);    // 0x440a30
    virtual Function *Create(ArgList *list);             // 0x440940
    virtual void Print();                                // 0x4409f0
    virtual const char *Name();                          // 0x440910
    StringFunctionArgument *first;                       // 0x04
    StringFunctionArgument *second;                      // 0x08
};
