// RemoveCurrentGoodieFunction::Execute (0x440b40): take the goodie the creature is holding out of the level.
#include "FunctionMisc/MiscClasses.h"

struct RgLevel { void RemoveGoodie(int id); };           // 0x44e020
struct RgPlace { RgLevel *GetLevel(); };                 // 0x439b50
struct RgInventory {
    char pad[8];
    String name;                                         // 0x08
    int current;                                         // 0x0c (overlaps the String slot's neighbour in the original layout)
    inline void Clear() { name = String(""); current = -1; }
};
struct RgOwner { RgInventory *Inventory(); };            // 0x448990
struct RgActor {
    char pad[0x6c];
    RgPlace *place;                                      // 0x70
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16(); virtual void s17();
    virtual RgOwner *Soul();                             // slot 18 (+0x48)
};
struct RgCall { RgActor *Actor(); };                     // 0x420c20

int RemoveCurrentGoodieFunction::Execute(ScriptCall *args, int extra)
{
    int goodie = ((RgCall *)args)->Actor()->Soul()->Inventory()->current;
    if (goodie != -1) {
        RgActor *actor = ((RgCall *)args)->Actor();
        actor->place->GetLevel()->RemoveGoodie(goodie);
        ((RgCall *)args)->Actor()->Soul()->Inventory()->Clear();
    }
    return 1;
}
