// CreatePropFunction base slots (SQZ.EXE 0x46f870-0x46fb00).
#include "FunctionProp/Prop.h"

int CreatePropFunction::ParseExtra(ArgList *list) { return 1; }
void CreatePropFunction::DumpExtra() {}
int CreatePropFunction::Apply(void *a, void *b, void *c) { __asm { } }

CreatePropFunction::CreatePropFunction()
{
    name = 0;
    position = 0;
    direction = 0;
}
