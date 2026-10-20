// 4 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
// AnimateCameraPathFunction::Execute (0x44e560).
#include "FunctionCamPath/CamPath.h"

int AnimateCameraPathFunction::Execute(ScriptCall *args, int extra)
{
    if (FindPath(path.c_str())) {
        void *soul = GetSimulation()->GetSoul(1);
        CameraSoul *cam = (CameraSoul *)DynamicCast(soul, 0, kSoulType, kCameraSoulType, 0);
        cam->FollowPath(FindPath(path.c_str()), a->Get(extra), b->Get(extra), c->Get(extra));
        return 1;
    }
    DebugLog("Error:  path %s does not exist.\n", path.c_str());
    return 0;
}
