// SqueezilSoul::Touch (slot 4, SQZ.EXE 0x450430, ret 4): a creature touched this soul; the touching one's grant interface decides, else a physical bump.
struct ActorHolder;
struct Taker {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual int Slot3(int flag);                         // +0xc
    virtual void v4();
    virtual int Slot5(int id);                           // +0x14
};
struct OtherSoul { virtual void v0(); virtual Taker *GetInterface(int which); };   // slot 1 (+4)
struct Sim {
    OtherSoul *GetSoul(int id);                          // 0x414080
    int FindIdBySoul(void *soul);                        // 0x4140b0
    int FindIdByActor(void *actor);                      // 0x414100
};
Sim *GetSimulation();                                    // 0x420690 (cdecl)
void *DynamicCast(void *p, int a, void *from, void *to, int b);   // 0x48f060
void PhysicalCollision(int a, int b);                    // 0x447590 (cdecl)
extern char kMessageMapType[], kGrantMessageMapType[];
struct SoulT {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
    virtual void s11();
    virtual void s12();
    virtual void s13();
    virtual void s14();
    virtual void s15();
    virtual void s16();
    virtual struct ActorHolder *GetBody();
    void Touch(int otherId);
};

void SoulT::Touch(int otherId)
{
    Sim *sim = GetSimulation();
    Taker *taker = (Taker *)DynamicCast(sim->GetSoul(otherId)->GetInterface(1), 0, kMessageMapType, kGrantMessageMapType, 0);
    int id = sim->FindIdBySoul(this);
    if (taker) {
        if (taker->Slot3(id != 0)) {
            if (!taker->Slot5(id))
                PhysicalCollision(sim->FindIdByActor(GetBody()), otherId);
            return;
        }
        PhysicalCollision(sim->FindIdByActor(GetBody()), otherId);
    }
}
