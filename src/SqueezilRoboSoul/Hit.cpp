#include "Runtime/DynamicCast.h"
// SqueezilRoboSoul::Hit (slot 9, SQZ.EXE 0x404630): pick the activity for being hit by this kind of creature, run it, then the base Hit.
struct Body2 {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14();
    virtual int IsOnGround();                            // slot 15 (+0x3c)
};
struct KindSoul { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
                  virtual void v6(); virtual void v7();
                  virtual int Kind(); };                 // slot 8 (+0x20)
struct SoulHolder { KindSoul *GetSoul(int id); };        // 0x414080
SoulHolder *GetSimulation();                             // 0x420690 (cdecl)
extern char kSoulType[], kKindSoulType[];
struct Character {
    int TrySetState(int state, void *soul);              // 0x4486f0
    void RunPending(void *soul);                         // 0x4487f0
};
struct SqueezilSoul { void Hit(int knock, int sprayNuts, int byId); };   // 0x4506d0
struct SqueezilRoboSoul {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16();
    virtual Body2 *GetBody();                            // slot 17 (+0x44)
    char pad[0x9c - 4];
    int senseKind;                                       // 0x9c
    char pad2[0xf4 - 0xa0];
    Character *character;                                // 0xf4
    void Hit(int knock, int sprayNuts, int byId);
};

void SqueezilRoboSoul::Hit(int knock, int sprayNuts, int byId)
{
    senseKind = ((KindSoul *)SqzRTDynamicCast(GetSimulation()->GetSoul(byId), 0, kSoulType, kKindSoulType, 0))->Kind();
    int wanted = 0x13;
    int onGround = GetBody()->IsOnGround();
    switch (senseKind) {
    case 0: wanted = onGround ? 0xd : 0xe; break;
    case 1: wanted = onGround ? 0xf : 0x10; break;
    case 2: wanted = onGround ? 0x11 : 0x12; break;
    }
    if (wanted != 0x13 && character->TrySetState(wanted, this))
        character->RunPending(this);
    ((SqueezilSoul *)(void *)this)->Hit(knock, sprayNuts, byId);
}
