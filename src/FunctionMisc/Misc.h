// The simple script commands that take no operand (derive straight from Function).
#pragma once
#include "FunctionArgs/FunctionArgs.h"

class GoToRegionFunction : public Function {
public:
    virtual int Execute(ScriptCall *args, int extra);   // 0x441630
    virtual Function *Create(ArgList *list);            // 0x4425a0
    virtual void Print();                               // 0x442630
    virtual const char *Name();                         // 0x442650
};
