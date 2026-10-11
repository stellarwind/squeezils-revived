// FlashGoodieFunction (vtable 0x49e7bc): make a goodie icon flash on the HUD.
#pragma once
#include "FunctionArgs/FunctionArgs.h"
#include "FunctionMisc/MiscClasses.h"

struct FlashCall {                       // ScriptCall seen through its accessors
    void *Actor();                       // 0x420c20
    int Busy();                          // 0x420c60
};
struct FlashHud { void FlashGoodie(int kind, float seconds); };   // 0x42e6f0
FlashHud *GetFlashHud();                 // 0x4206c0 (cdecl)

class FlashGoodieFunction : public Function {
public:
    FlashGoodieFunction() { kind = 0; seconds = 0; }
    ~FlashGoodieFunction();                              // 0x441670
    virtual int Execute(ScriptCall *args, int extra);    // 0x441840
    virtual Function *Create(ArgList *list);             // 0x441750
    virtual void Print();                                // 0x441800
    virtual const char *Name();                          // 0x441720
    IntFunctionArgument *kind;                           // 0x04
    FloatFunctionArgument *seconds;                      // 0x08
};
