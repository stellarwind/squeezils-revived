// 0x44e7f0: registers the two camera-path script functions.
#include "FunctionCamPath/CamPath.h"
void RegisterFunction(Function *f);                      // 0x421d00 (cdecl)

void RegisterCameraPathFunctions()
{
    RegisterFunction(new AnimateCameraPathFunction);
    RegisterFunction(new AnimateCameraPathOffFunction);
}
