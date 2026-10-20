// 3 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
// CompoundFunction::Print (0x421c40).
#include "FunctionCompound/Compound.h"

void CompoundFunction::Print()
{
    DebugLogAll("compound");
    for (int i = 0; i < items.count; i++) {
        DebugLogAll("  ");
        items.Get(i)->Print();
    }
}
