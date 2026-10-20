// AnimateCameraPathOffFunction::Execute (0x44e790): switch the camera path off when none is running.
#include "FunctionMisc/MiscClasses.h"

struct CamSim { void *GetSoul(int id); };                // 0x414080
CamSim *GetSimulation();                                 // 0x420690 (cdecl)
void *DynamicCast(void *p, int zero, const void *from, const void *to, int ref);   // 0x48f060 (cdecl)
extern const char kSoulType[];                           // 0x4b93e8
extern const char kCameraSoulType[];                     // 0x4b98b0
struct CamSoul {
    char pad[0x14];
    int running;                                         // 0x14
    void SwitchOff(float seconds);                       // 0x409c40
};

int AnimateCameraPathOffFunction::Execute(ScriptCall *args, int extra)
{
    void *soul = GetSimulation()->GetSoul(1);
    CamSoul *cam = (CamSoul *)DynamicCast(soul, 0, kSoulType, kCameraSoulType, 0);
    if (!cam->running)
        cam->SwitchOff(arg->Get(extra));
    return 1;
}
