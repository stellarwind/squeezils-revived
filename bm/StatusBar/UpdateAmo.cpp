// StatusBar::UpdateAmo, SQZ.EXE 0x42eec0.
#include "StatusBar/StatusBar.h"

struct AmmoInventory { int Count(const char *name); }; // 0x416300
struct AmmoOwner { AmmoInventory *Inventory(); }; // 0x448990
struct AmmoActor {
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8();
    virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16(); virtual void s17();
    virtual AmmoOwner *Soul(); // slot 18
};
struct AmmoSimulation { AmmoActor *GetSoul(int id); }; // 0x414080
AmmoSimulation *GetSimulation(); // 0x420690

int StatusBar::UpdateAmo(int force)
{
    AmmoActor *actor = GetSimulation()->GetSoul(0);
    if (!actor)
        return 0;
    AmmoOwner *owner = actor->Soul();
    if (!owner)
        return 0;
    amoCount = owner->Inventory()->Count("SpitNutGoodie");
    if (force || amoCount != amoShown)
        amo.Draw(amoCount, 0);
    int changed = amoShown != amoCount;
    amoShown = amoCount;
    return changed;
}
