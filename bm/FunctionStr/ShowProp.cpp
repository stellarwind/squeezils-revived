// 11 spare declarations (symbol count, see padscan.sh)
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
// ShowPropFunction::Execute (0x470070).
#include "FunctionStr/PropExec.h"

int ShowPropFunction::Execute(ScriptCall *args, int extra)
{
    PropV *p = FindPropV(text, ((PropLevel *)args)->Level(), 1);
    int a = p->Shown();
    int b = p->ShowKind();
    if (b)
        p->Show(b);
    else if (!a)
        p->Show(g_notifier->NextKind());
    return 1;
}
