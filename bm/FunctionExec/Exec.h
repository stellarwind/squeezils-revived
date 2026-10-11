// ExecuteFunction (vtable 0x49e8e4): run another script function by name.
#pragma once
#include "FunctionArgs/FunctionArgs.h"

class StringFunctionArgument : public FunctionArgument {   // vtable 0x49da9c
public:
    StringFunctionArgument();
    virtual void Print();                                // 0x421a70
    String text;                                         // 0x0c
    String Get(int context);                             // 0x421a90 (by value)
};
StringFunctionArgument *ParseStringArgument(ArgList *list);   // 0x421940 (cdecl)
int PostFunction(const char *name, void *args, int extra, int local);   // 0x422ce0 (cdecl)

class ExecuteFunction : public Function {
public:
    ExecuteFunction();                                   // 0x444770
    ~ExecuteFunction();                                  // 0x4447e0 sdd calls it
    virtual int Execute(ScriptCall *args, int extra);    // 0x444960
    virtual Function *Create(ArgList *list);             // 0x4448a0
    virtual void Print();                                // 0x444930
    virtual const char *Name();                          // 0x4447d0
    StringFunctionArgument *name;                        // 0x04
};
