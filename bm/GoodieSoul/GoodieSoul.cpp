// GoodieSoul getters (0x459960-0x45a840).
#include "GoodieSoul/GoodieSoul.h"

extern void *g_goodieSettings;   // 0x4c7bb4 (points to the static GoodieSettings)

void *GoodieSoul::GetInterface(int which)
{
    if (which == 1)
        return this ? (char *)this + 4 : 0;
    return 0;
}

int GoodieSoul::IsLoose()    { return kind == 0; }
int GoodieSoul::GetKind()    { return kind == 0 ? 2 : 4; }
int GoodieSoul::IsAttacking(){ return thrown != 0; }
void *GoodieSoul::GetSettings() { return g_goodieSettings; }

int GoodieSoulGrant::GrantSlot3(int arg)
{
    if (kind != 0)
        return arg == 0;
    return thrown == 0;
}
