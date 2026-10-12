// GoodieSoul::Touch (slot 4, SQZ.EXE 0x4599b0): a creature touched the goodie; one that takes goodies is given it (unless thrown).
#include "String/String.h"
struct Taker {
    virtual void v0(); virtual void v1();
    virtual int Accepts(int kind);                       // slot 2 (+8)
    virtual void v3();
    virtual int Grant(const char *name, int id);         // slot 4 (+0x10)
};
struct OtherSoul { virtual void v0(); virtual Taker *GetInterface(int which); };   // slot 1 (+4)
struct Sim {
    OtherSoul *GetSoul(int id);                          // 0x414080
    int FindIdBySoul(void *soul);                        // 0x4140b0
    int FindIdByActor(void *actor);                      // 0x414100
    void AddId(int id);                                  // 0x414180
};
Sim *GetSimulation();                                    // 0x420690 (cdecl)
void *DynamicCast(void *p, int a, void *from, void *to, int b);   // 0x48f060
void PhysicalCollision(int a, int b);                    // 0x447590 (cdecl)
extern char kMessageMapType[], kGrantMessageMapType[];
struct GoodieSoul {
    void **vtbl, **grantVtbl;
    void *body;                                          // 0x08
    String name;                                         // 0x0c
    int kind;                                            // 0x10 (see below)
    int thrown;                                          // 0x14
    void Touch(int otherId);
};

void GoodieSoul::Touch(int otherId)
{
    Sim *sim = GetSimulation();
    Taker *taker = (Taker *)DynamicCast(sim->GetSoul(otherId)->GetInterface(1), 0, kMessageMapType, kGrantMessageMapType, 0);
    if (!taker)
        return;
    if (taker->Accepts(*(int *)((char *)this + 0x10)) && !thrown) {
        if (taker->Grant(name.c_str(), sim->FindIdBySoul(this))) {
            sim->AddId(sim->FindIdBySoul(this));
            return;
        }
        PhysicalCollision(sim->FindIdByActor(body), otherId);
        return;
    }
    PhysicalCollision(sim->FindIdByActor(body), otherId);
    thrown = 0;
}
