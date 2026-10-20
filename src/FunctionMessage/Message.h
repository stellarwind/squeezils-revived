// MessageFunction (vtable 0x49e6d4) and TimerMessageFunction (0x49e6ec): show a text message (now / after a delay).
#pragma once
#include "FunctionArgs/FunctionArgs.h"

struct MsgArgList {                      // the argument list of a Message command: a list of words
    virtual String Item(int i);          // slot 0
    int count;                           // 0x04
};
struct Hud { void ShowMessage(const char *text); void ShowTimerMessage(const char *text); };   // 0x42dd30 / 0x42e0e0
Hud *GetHud();                           // 0x4206c0 (cdecl)

class MessageFunction : public Function {
public:
    MessageFunction() {}
    virtual int Execute(ScriptCall *args, int extra);    // 0x440090
    virtual Function *Create(ArgList *list);             // 0x43fde0
    virtual void Print();                                // 0x440060
    virtual const char *Name();                          // 0x43ffd0
    String text;                                         // 0x04
};

class TimerMessageFunction : public Function {
public:
    TimerMessageFunction() {}
    virtual int Execute(ScriptCall *args, int extra);    // 0x440360
    virtual Function *Create(ArgList *list);             // 0x4400b0
    virtual void Print();                                // 0x440330
    virtual const char *Name();                          // 0x4402a0
    String text;                                         // 0x04
};
