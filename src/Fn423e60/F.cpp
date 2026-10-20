// 0x423e60: registers four script functions (robo behaviour / response, spit nuts, sense targets).
#include "FunctionRobo/Robo.h"
#include "FunctionSpit/Spit.h"
#include "FunctionMisc/Noarg2.h"
void RegisterFunction(Function *f);                      // 0x421d00 (cdecl)

void RegisterRoboFunctions()
{
    RegisterFunction(new SetRoboBehaviorFunction);
    RegisterFunction(new ChangeRoboResponseFunction);
    RegisterFunction(new AddSpitNutsFunction);
    RegisterFunction(new ClearSenseTargetsFunction);
}
