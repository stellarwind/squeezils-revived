// ClearMessage::Execute (0x4406c0) and ClearTimerMessage::Execute (0x440810): show the empty string.
#include "FunctionMisc/Noarg2.h"
#include "FunctionMisc/MiscClasses.h"
#include "FunctionMessage/Message.h"

int ClearMessageFunction::Execute(ScriptCall *args, int extra)
{
    GetHud()->ShowMessage("");
    return 1;
}

int ClearTimerMessageFunction::Execute(ScriptCall *args, int extra)
{
    GetHud()->ShowTimerMessage("");
    return 1;
}
