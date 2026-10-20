// CreatePropFunction::Print (0x46fab0).
#include "FunctionProp/Prop.h"

void CreatePropFunction::Print()
{
    DebugLogAll("%s", Name());
    name->Print();
    position->Print();
    direction->Print();
    DumpExtra();
    DebugLogAll("\n");
}
