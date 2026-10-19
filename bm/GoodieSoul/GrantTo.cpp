// @00459b80 GrantMessageMap slot 5; this points four bytes into GoodieSoul.
#include "../String/String.h"

struct Taker {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual int Grant(const char *name, int id);          // slot 4
};
struct OtherSoul {
    virtual void v0();
    virtual Taker *GetInterface(int which);              // slot 1
};
struct Sim {
    OtherSoul *GetSoul(int id);                          // 0x414080
    int FindIdBySoul(void *soul);                        // 0x4140b0
    void AddId(int id);                                  // 0x414180
};
Sim *GetSimulation();                                    // 0x420690

struct GoodieSoulGrant {
    char prefix[8];                                      // grant vtable, body
    String name;                                        // soul + 0x0c
    int GrantTo(int otherId);
};

int GoodieSoulGrant::GrantTo(int otherId)
{
    Sim *sim;
    void *soul;
    Taker *taker;

    sim = GetSimulation();
    taker = sim->GetSoul(otherId)->GetInterface(1);
    soul = (char *)this - 4;
    if (taker->Grant(name.c_str(), sim->FindIdBySoul(soul))) {
        sim->AddId(sim->FindIdBySoul(soul));
        return 1;
    }
    return 0;
}
