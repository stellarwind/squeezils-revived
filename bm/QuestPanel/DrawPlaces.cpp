// QuestPanel::DrawPlaces, SQZ.EXE 0x42f6d0.
#include "QuestPanel/QuestPanel.h"

struct QPInventory { int Count(const char *name); }; // 0x416300
struct QPOwner { QPInventory *Inventory(); };        // 0x448990
struct QPActor {
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8();
    virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16(); virtual void s17();
    virtual QPOwner *Soul();                         // slot 18
};
struct QPSimulation { QPActor *GetSoul(int id); };    // 0x414080
QPSimulation *GetSimulation();                       // 0x420690

int QuestPanel::DrawPlaces(int force)
{
    if (!force)
        return 0;
    int gold = 0, diamond = 0;
    QPActor *actor = GetSimulation()->GetSoul(0);
    if (actor) {
        QPOwner *owner = actor->Soul();
        if (owner) {
            gold = owner->Inventory()->Count("GoldPlacesCompleted");
            diamond = owner->Inventory()->Count("DiamondPlacesCompleted");
        }
    }
    if (force)
        places.Draw(gold, diamond);
    return 0;
}
