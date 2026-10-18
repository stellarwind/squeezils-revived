// StatusBar::UpdateFood, SQZ.EXE 0x42ef50.
// Three spare declarations retain the closest VC++ 4.1 instruction schedule.
void FoodPad1();
void FoodPad2();
void FoodPad3();
#include "StatusBar/StatusBar.h"
#include "String/String.h"

struct FoodInventory { int Count(const char *name); }; // 0x416300
struct FoodOwner { FoodInventory *Inventory(); }; // 0x448990
struct FoodActor {
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8();
    virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16(); virtual void s17();
    virtual FoodOwner *Soul(); // slot 18
};
struct FoodSimulation { FoodActor *GetSoul(int id); }; // 0x414080
FoodSimulation *GetSimulation(); // 0x420690
struct FoodSettings {
    virtual void Set(const String &name, int *value, int flags); // slot 0
};
extern FoodSettings gFoodSettings; // 0x4c7060

int StatusBar::UpdateFood(int force)
{
    FoodActor *actor = GetSimulation()->GetSoul(0);
    if (!actor)
        return 0;
    FoodOwner *owner = actor->Soul();
    if (!owner)
        return 0;
    int *count = &foodCount;
    *count = owner->Inventory()->Count("FoodNutGoodie");
    if (force || *count != foodShown) {
        food.Draw(*count, foodGoal);
        FoodSettings *settings = &gFoodSettings;
        settings->Set(String("PlayerFoodNuts"), count, -1);
    }
    int changed = foodShown != *count;
    foodShown = *count;
    return changed;
}
