// TweakFloatFunction (vtable 0x49de44) and its twenty subclasses: physics tweaks that take a list of float arguments.
#pragma once
#include "FunctionArgs/FunctionArgs.h"
#include "Container/ContainerArray.h"

typedef FunctionArgument FloatFunctionArgument;     // vtable 0x49da84 in the original (Print = 0x421410)
FloatFunctionArgument *ParseFloatArgument(ArgList *list);   // 0x421250 (cdecl)

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
