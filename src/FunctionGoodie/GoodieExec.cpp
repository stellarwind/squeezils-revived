#include "Runtime/DynamicCast.h"
// CreateGoodieAtPositionFunction::Execute (0x440fa0, shared with the WithVelocity subclass): build the spawn transform and hand it to slot 8.
#define GOODIE_EXEC
#include "FunctionGoodie/Goodie.h"

struct GoodiePlace { void *GetLevel(); };                // PlaceSimulation::GetLevel 0x439b50
struct SimRoot { int dummy; };
SimRoot *GetSim();                                       // 0x420690 (cdecl)
extern const char kSimType[];                            // 0x4ba1d0
extern const char kPlaceSimType[];                       // 0x4bae38

int CreateGoodieAtPositionFunction::Execute(ScriptCall *args, int extra)
{
    GoodieMatrix m;
    Vector3f right = direction->Get(extra).Cross(Vector3f(0, 0, 1));
    right.Normalize();
    Vector3f up = Vector3f(0, 0, 1).Cross(right);
    up.Normalize();
    m.Set(right, up, Vector3f(0, 0, 1), position->Get(extra));
    GoodiePlace *place = (GoodiePlace *)SqzRTDynamicCast(GetSim(), 0, kSimType, kPlaceSimType, 0);
    if (!place)
        return 1;
    Spawn(extra, place->GetLevel(), name->Get(extra).c_str(), &m);
    return 1;
}
