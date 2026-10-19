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
class IfVariableNonZeroFunction : public IfVariableFunction {
public:
    virtual int Check(int value, ScriptCall *args, int extra);
    virtual void Print();
    virtual const char *Name();
};
void RegisterFunction(Function *f);                      // 0x421d00 (cdecl)

void RegisterFunctionsC()
{
    RegisterFunction(new IfOnSurfaceTypeFunction);
    RegisterFunction(new IfOnOrAboveSurfaceTypeFunction);
    RegisterFunction(new IfOnFaceSetFunction);
    RegisterFunction(new IfOnTreeFunction);
    RegisterFunction(new IfPlayerDeadFunction);
    RegisterFunction(new IfPlayerDeadFunction);
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
    RegisterFunction(new AbortFunction);
    RegisterFunction(new AbortFunction);
    RegisterFunction(new AbortFunction);
    RegisterFunction(new RenderDelayedExecuteFunction);
}
