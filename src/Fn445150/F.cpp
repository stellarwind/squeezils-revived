// Declaration padding preserves VC4 code after sharing script dispatch.
void DispatchPad000();
void DispatchPad001();
void DispatchPad002();
void DispatchPad003();
void DispatchPad004();
void DispatchPad005();
void DispatchPad006();
void DispatchPad007();
void DispatchPad008();
void DispatchPad009();
void DispatchPad010();
void DispatchPad011();
void DispatchPad012();
void DispatchPad013();
void DispatchPad014();
void DispatchPad015();
void DispatchPad016();
void DispatchPad017();
void DispatchPad018();
// 0x445150: registers the script functions of this group (new X + RegisterFunction, in order).
#include "FunctionExec/Delayed.h"
#include "FunctionIf/IfClasses.h"
#define StringFunctionArgument StringFunctionArgument_S
#define ParseStringArgument ParseStringArgument_S
#include "FunctionIf/IfSurface.h"
#undef StringFunctionArgument
#undef ParseStringArgument
#include "FunctionIf/IfVar.h"
#include "FunctionMisc/Noarg2.h"
#include "FunctionSet/Set.h"
#include "FunctionWithProp/WithProp.h"
// SIZE 0x0c
class IfVariableNonZeroFunction : public IfVariableFunction {
public:
    virtual int Check(int value, ScriptCall *args, int extra); // vtable+0x24
    virtual const char *Name();                              // vtable+0x10
    virtual IfFunction *Make();                              // vtable+0x20
};
void RegisterFunction(Function *f);                      // 0x421d00 (cdecl)

void RegisterFunctionsC()
{
    RegisterFunction(new IfOnSurfaceTypeFunction);
    RegisterFunction(new IfOnOrAboveSurfaceTypeFunction);
    RegisterFunction(new IfOnFaceSetFunction);
    RegisterFunction(new IfOnTreeFunction);
    RegisterFunction(new IfPlayerDeadFunction);
    RegisterFunction(new IfPlayerFunction);
    RegisterFunction(new IfHasAllKeysFunction);
    RegisterFunction(new IfCompletedPlaceFunction);
    RegisterFunction(new IfVariableNonZeroFunction);
    RegisterFunction(new IfVariableGreaterFunction);
    RegisterFunction(new IfVariableLesserFunction);
    RegisterFunction(new IfVariableEqualFunction);
    RegisterFunction(new IfDyingOrDeadFunction);
    RegisterFunction(new AddToFunction);
    RegisterFunction(new SetToFunction);
    RegisterFunction(new AbortFunction);
    RegisterFunction(new ExecuteFunction);
    RegisterFunction(new ExecuteWithPropFunction);
    RegisterFunction(new DelayedExecuteFunction);
    RegisterFunction(new RenderDelayedExecuteFunction);
}
