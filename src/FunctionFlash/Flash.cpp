// FlashGoodieFunction (SQZ.EXE 0x441670-0x4418a0).
#include "FunctionFlash/Flash.h"

FlashGoodieFunction::~FlashGoodieFunction()
{
    if (kind)
        delete kind;
    if (seconds)
        delete seconds;
}

int FlashGoodieFunction::Execute(ScriptCall *args, int extra)
{
    FlashCall *c = (FlashCall *)args;
    if (c->Actor() && !c->Busy())
        GetFlashHud()->FlashGoodie(kind->Get(extra), seconds->Get(extra));
    return 1;
}

void FlashGoodieFunction::Print()
{
    DebugLogAll("%s", Name());
    kind->Print();
    seconds->Print();
    DebugLogAll("\n");
}

const char *FlashGoodieFunction::Name() { return "FlashGoodie"; }
