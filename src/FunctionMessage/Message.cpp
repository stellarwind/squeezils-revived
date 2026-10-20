// MessageFunction / TimerMessageFunction (SQZ.EXE 0x43fde0-0x440380).
#include <string.h>
#include "FunctionMessage/Message.h"

Function *MessageFunction::Create(ArgList *list)
{
    MessageFunction *f = new MessageFunction;
    MsgArgList *l = (MsgArgList *)list;
    int total = 0;
    int i = 0;
    char buf[160];
    buf[0] = 0;
    for (; i < l->count; i++) {
        total += strlen(l->Item(i).c_str()) + 1;
        strcat(buf, l->Item(i).c_str());
        strcat(buf, " ");
    }
    f->text = String(buf);
    return f;
}

int MessageFunction::Execute(ScriptCall *args, int extra)
{
    GetHud()->ShowMessage(text.c_str());
    return 1;
}

void MessageFunction::Print()
{
    DebugLogAll("%s %s", Name(), text.c_str());
}

const char *MessageFunction::Name() { return "Message"; }

Function *TimerMessageFunction::Create(ArgList *list)
{
    TimerMessageFunction *f = new TimerMessageFunction;
    MsgArgList *l = (MsgArgList *)list;
    int total = 0;
    int i = 0;
    char buf[160];
    buf[0] = 0;
    for (; i < l->count; i++) {
        total += strlen(l->Item(i).c_str()) + 1;
        strcat(buf, l->Item(i).c_str());
        strcat(buf, " ");
    }
    f->text = String(buf);
    return f;
}

int TimerMessageFunction::Execute(ScriptCall *args, int extra)
{
    GetHud()->ShowTimerMessage(text.c_str());
    return 1;
}

void TimerMessageFunction::Print()
{
    DebugLogAll("%s %s", Name(), text.c_str());
}

const char *TimerMessageFunction::Name() { return "TimerMessage"; }
