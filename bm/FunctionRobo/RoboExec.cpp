// ChangeRoboResponseFunction::Execute (0x423a50).
#include "FunctionRobo/Robo.h"

struct RoboResponses { void SetResponse(int kind, String response); };   // 0x448640
struct RoboCreature {
    char pad[0xf4];
    RoboResponses *responses;                            // 0xf4
};
struct RoboCall {                                        // ScriptCall seen through its accessors
    RoboCreature *Creature();                            // 0x420c50
    int Other();                                         // 0x420c60
};

int ChangeRoboResponseFunction::Execute(ScriptCall *args, int extra)
{
    RoboCreature *robo = ((RoboCall *)args)->Creature();
    if (!robo) {
        DebugLog("WARNING: attempt to run ChangeRoboResponse on non-Robo creature (id %d).\n", ((RoboCall *)args)->Other());
        return 0;
    }
    robo->responses->SetResponse(kind->Resolve(0), response->Get(0));
    return 1;
}
