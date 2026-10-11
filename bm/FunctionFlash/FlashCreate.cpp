// 16 spare declarations (symbol count, see padscan.sh)
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
void PadDecl15();
void PadDecl16();
// FlashGoodieFunction::Create (0x441750).
#include "FunctionFlash/Flash.h"

Function *FlashGoodieFunction::Create(ArgList *list)
{
    FlashGoodieFunction *f = new FlashGoodieFunction;
    f->kind = (IntFunctionArgument *)ParseArgument(list);
    f->seconds = ParseFloatArgument(list);
    return f;
}
