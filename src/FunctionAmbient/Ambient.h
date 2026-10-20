// StartAmbientLoopFunction (vtable 0x49e2c4): start a looping ambient sound.
#pragma once
#include "FunctionArgs/FunctionArgs.h"
#include "FunctionMisc/MiscClasses.h"

struct AmbArgList {
    virtual String ReadString(int flag);                 // slot 0
    virtual void a1();
    virtual void Skip(int n);                            // slot 2
};

class StartAmbientLoopFunction : public Function {
public:
    StartAmbientLoopFunction() { a = 0; b = 0; c = 0; }
    ~StartAmbientLoopFunction() { delete a; delete b; delete c; }
    virtual int Execute(ScriptCall *args, int extra);    // 0x43a4d0
    virtual Function *Create(ArgList *list);             // 0x43a230
    virtual void Print();                                // 0x43a480
    virtual const char *Name();                          // 0x43a360
    String sound;                                        // 0x04
    IntFunctionArgument *a;                              // 0x08
    FloatFunctionArgument *b;                            // 0x0c
    FloatFunctionArgument *c;                            // 0x10
};
