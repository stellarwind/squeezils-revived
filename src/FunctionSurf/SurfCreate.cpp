// 5 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
// SetSurfaceTypeFunction::Create (0x440940).
#include "FunctionSurf/Surf.h"

Function *SetSurfaceTypeFunction::Create(ArgList *list)
{
    SetSurfaceTypeFunction *f = new SetSurfaceTypeFunction;
    f->first = ParseStringArgument(list);
    f->second = ParseStringArgument(list);
    return f;
}
