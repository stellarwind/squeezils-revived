// CreatePropAtPosition slots 6-8 (SQZ.EXE 0x46fdd0-0x46fe90).
#define V3_CTOR_OUTOFLINE
#define V3_COPYCTOR
#include "FunctionProp/PropClasses.h"

void CreatePropAtPositionFunction::DumpExtra() { offset->Print(); }

Vector3f CreatePropAtPositionFunction::Position(ScriptCall *args, int extra)
{
    return offset->Get(extra);
}
