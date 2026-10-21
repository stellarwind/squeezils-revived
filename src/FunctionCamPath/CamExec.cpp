#ifdef SQZ_INTEGRATION
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
void DispatchPad019();
void DispatchPad020();
void DispatchPad021();
void DispatchPad022();
void DispatchPad023();
void DispatchPad024();
void DispatchPad025();
void DispatchPad026();
void DispatchPad027();
void DispatchPad028();
void DispatchPad029();
void DispatchPad030();
#else
// Matching build retains its original register allocation.
void PadDecl001();
void PadDecl002();
void PadDecl003();
void PadDecl004();
void PadDecl005();
void PadDecl006();
void PadDecl007();
void PadDecl008();
void PadDecl009();
void PadDecl010();
void PadDecl011();
void PadDecl012();
void PadDecl013();
void PadDecl014();
void PadDecl015();
void PadDecl016();
void PadDecl017();
void PadDecl018();
void PadDecl019();
void PadDecl020();
void PadDecl021();
void PadDecl022();
void PadDecl023();
void PadDecl024();
void PadDecl025();
void PadDecl026();
void PadDecl027();
#endif
#include "Runtime/DynamicCast.h"
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
        CameraSoul *cam = (CameraSoul *)SqzRTDynamicCast(soul, 0, kSoulType, kCameraSoulType, 0);
        cam->FollowPath(FindPath(path.c_str()), a->Get(extra), b->Get(extra), c->Get(extra));
        return 1;
    }
    DebugLogAll("Error:  path %s does not exist.\n", path.c_str());
    return 0;
}
