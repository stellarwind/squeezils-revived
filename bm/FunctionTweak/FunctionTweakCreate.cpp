// TweakFloatFunction::Create (0x430620), alone in its translation unit: its register choice needs 8 spare declarations (padscan.sh).
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
#include "FunctionTweak/FunctionTweak.h"

Function *TweakFloatFunction::Create(ArgList *list)
{
    TweakFloatFunction *f = Make();
    for (int i = 0; i < ArgCount(); i++) {
        FloatFunctionArgument *a = ParseFloatArgument(list);
        f->args.Insert(a, -1);
    }
    return f;
}
