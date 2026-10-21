// CreatureTemplate::RunPending, SQZ.EXE 0x4487f0.
#include "CreatureTemplate/CreatureTemplate.h"

struct PendingSoul {
    char pad[0xf8];
    const char *pendingFunction;
};
struct PendingCaller {
    char storage[0x18];
    PendingCaller(PendingSoul *soul, int id, int zero); // 0x420a60
};
struct PendingSimulation {
    int FindIdBySoul(PendingSoul *soul); // 0x4140b0
};
PendingSimulation *GetSimulation(); // 0x420690
#include "Script/CallFunction.h"

void CreatureTemplate::RunPending(PendingSoul *soul)
{
    if (soul->pendingFunction) {
        PendingCaller caller(soul, GetSimulation()->FindIdBySoul(soul), 0);
        CallFunction(soul->pendingFunction, &caller, 0, 0);
        soul->pendingFunction = 0;
    }
}
