// 0x4311c0: registers the script functions of this group (new X + RegisterFunction, in order).
#include "FunctionTweak/TweakClasses.h"
void RegisterFunction(Function *f);                      // 0x421d00 (cdecl)

void RegisterTweakFunctions()
{
    RegisterFunction(new SetFrictionFunction);
    RegisterFunction(new ResetFrictionFunction);
    RegisterFunction(new SetClawsFunction);
    RegisterFunction(new SetClawsZeroFunction);
    RegisterFunction(new ResetClawsFunction);
    RegisterFunction(new SetWalkSpeedsFunction);
    RegisterFunction(new ResetWalkSpeedsFunction);
    RegisterFunction(new SetJumpFunction);
    RegisterFunction(new ResetJumpFunction);
    RegisterFunction(new SetJumpableFunction);
    RegisterFunction(new ResetJumpableFunction);
    RegisterFunction(new SetHopFunction);
    RegisterFunction(new ResetHopFunction);
    RegisterFunction(new SetMotionSpeedFunction);
    RegisterFunction(new LaunchFromFaceNormalFunction);
    RegisterFunction(new BounceBackwardsFunction);
    RegisterFunction(new AccelerateFunction);
}
