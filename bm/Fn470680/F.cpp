// 0x470680: registers the script functions of this group (new X + RegisterFunction, in order).
#include "FunctionCycle/CycleClasses.h"
#include "FunctionProp/PropClasses.h"
#include "FunctionStr/Str.h"
void RegisterFunction(Function *f);                      // 0x421d00 (cdecl)

void RegisterFunctionsD()
{
    RegisterFunction(new ShowPropFunction);
    RegisterFunction(new HidePropFunction);
    RegisterFunction(new SetCyclePropPathWrapFunction);
    RegisterFunction(new SetCyclePropPathDirFunction);
    RegisterFunction(new SetCyclePropPathTimeFunction);
    RegisterFunction(new SetCyclePropFrameWrapFunction);
    RegisterFunction(new SetCyclePropFrameDirFunction);
    RegisterFunction(new SetCyclePropFrameDirFunction);
    RegisterFunction(new CreatePropAtSqueezilPositionFunction);
    RegisterFunction(new CreatePropTrackingSqueezilPositionFunction);
    RegisterFunction(new CreatePropTrackingSqueezilXformFunction);
    RegisterFunction(new CreatePropAtPositionFunction);
}
