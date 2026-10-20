// 14 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
void PadDecl14();
// RenderDelayedExecuteFunction::Create (0x444f80).
#include "FunctionExec/Delayed.h"

Function *RenderDelayedExecuteFunction::Create(ArgList *list)
{
    RenderDelayedExecuteFunction *f = new RenderDelayedExecuteFunction;
    f->delay = ParseFloatArgument(list);
    f->name = ParseStringArgument(list);
    return f;
}
