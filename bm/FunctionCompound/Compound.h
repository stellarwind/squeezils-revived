// CompoundFunction (vtable 0x49daa4): a list of script functions run in order.
#pragma once
#include "FunctionArgs/FunctionArgs.h"
#include "Container/ContainerArray.h"

class CompoundFunction : public Function {
public:
    ~CompoundFunction();                                 // 0x421b00
    virtual int Execute(ScriptCall *args, int extra);    // 0x421bd0
    virtual Function *Create(ArgList *list);             // 0x421c30 (returns 0)
    virtual void Print();                                // 0x421c40
    virtual const char *Name();                          // 0x421ba0
    Array<Function *> items;                             // 0x04
};
