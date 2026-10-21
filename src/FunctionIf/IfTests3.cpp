#include "Runtime/DynamicCast.h"
// IfCompletedPlace's Test (0x4438b0).
#include "FunctionIf/IfClasses.h"

struct Sim { int dummy; };
Sim *GetSim();                                           // 0x420690 (cdecl)
extern const char kSimType[];                            // 0x4ba1d0
extern const char kPlaceType[];                          // 0x4ba370
struct Place { int Region(); int Piece(); };             // 0x418150 / 0x418160
struct Progress { int Completed(int region, int piece); };   // 0x41e9b0
Progress *GetProgress();                                 // 0x420680 (cdecl)

int IfCompletedPlaceFunction::Test(ScriptCall *args, int extra)
{
    Place *pl = (Place *)SqzRTDynamicCast(GetSim(), 0, kSimType, kPlaceType, 0);
    if (!pl)
        return 0;
    return GetProgress()->Completed(pl->Piece(), pl->Region());
}
