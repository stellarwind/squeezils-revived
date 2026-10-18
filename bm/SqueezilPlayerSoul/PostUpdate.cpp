// SqueezilPlayerSoul::PostUpdate (SQZ.EXE 0x41f750): once the nuts carried are enough to finish the level, run EnoughNutsFunction (once).
struct InvP { int Count(const char *kind); };               // 0x416300
struct InvOwnerP { InvP *Inventory(); };                    // 0x448990
struct PCaller {
    char storage[0x18];
    PCaller(struct PSoul *soul, int id, int zero);          // 0x420900
};
struct PSim { int FindIdBySoul(PSoul *soul); };             // 0x4140b0
PSim *GetSimulation();                                      // 0x420690
void PostFunction(const char *name, PCaller *caller, int first, int second);   // 0x422ce0
extern int g_foodNutsToFinish;                              // 0x4bad04

struct PSoul {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
    virtual InvOwnerP *GetOwner();                          // slot 18 (+0x48)
    char pad[0x94];
    int enoughNuts;                                         // 0x98
    int PostUpdate();
};

int PSoul::PostUpdate()
{
    if (enoughNuts)
        return 1;
    if (GetOwner()->Inventory()->Count("FoodNutGoodie") >= g_foodNutsToFinish) {
        PCaller caller(this, GetSimulation()->FindIdBySoul(this), 0);
        PostFunction("EnoughNutsFunction", &caller, 0, 0);
        enoughNuts = 1;
        return 1;
    }
    return 0;
}
