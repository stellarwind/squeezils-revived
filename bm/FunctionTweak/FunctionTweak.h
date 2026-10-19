// TweakFloatFunction (vtable 0x49de44) and its twenty subclasses: physics tweaks that take a list of float arguments.
#pragma once
#include "FunctionMisc/MiscClasses.h"
#include "Container/ContainerArray.h"

class TweakFloatFunction : public Function {
public:
    TweakFloatFunction() {}
    ~TweakFloatFunction();                                  // 0x430530
    virtual Function *Create(ArgList *list);                // slot 2: 0x430620
    virtual void Print();                                   // slot 3: 0x430690
    virtual TweakFloatFunction *Make() = 0;                 // slot 5: the subclass's factory
    virtual int ArgCount();                                 // slot 6: how many floats the command takes (0x4325d0 returns 0)
    Array<FloatFunctionArgument *> args;                    // 0x04
};
