// CreatePropFunction::Print (0x46fab0).
#include "FunctionProp/Prop.h"

void CreatePropFunction::Print()
{
    DebugLog("%s", Name());
    name->Print();
    position->Print();
    direction->Print();
    DumpExtra();
    DebugLog("\n");
}
