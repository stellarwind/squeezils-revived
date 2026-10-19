// SqueezilPlayerSoul constructor (SQZ.EXE 0x41ee30, ret 0xc): the SqueezilSoul part (0x44f1a0), the input-event base at +0x80, then the one-time [PlayerSleepTime]/[FoodNutsToFinish] reads.
#include "IniReader/IniReader.h"

struct SoulBodyP; struct CarriedP;
struct PhysBaseP { virtual ~PhysBaseP(); };
struct GrantP { virtual void g0(); };
struct SqSoulP : PhysBaseP, GrantP {
    char body[0x80 - 8];
    SqSoulP(SoulBodyP *b, CarriedP *p, void *pl);                  // 0x44f1a0
};
struct EventTargetP {                                              // vtable 0x49d2c4
    int a84; int a88; int a8c; int a90; int a94; int a98; int a9c; int aa0;
    EventTargetP() : a84(0), a90(0), a98(0), a9c(0) { a88 = -1; aa0 = 1; }
    virtual void Event();
};
extern int gSleepRead;                                             // 0x4bad00
extern float gSleepTime;                                           // 0x4c7058
extern int gFoodNuts;                                              // 0x4bad04
extern void *gFirstPlayer;                                         // 0x4bad08

struct SqueezilPlayerSoulC : SqSoulP, EventTargetP {
    SqueezilPlayerSoulC(SoulBodyP *b, CarriedP *p, void *pl);
    virtual ~SqueezilPlayerSoulC();
};

SqueezilPlayerSoulC::SqueezilPlayerSoulC(SoulBodyP *b, CarriedP *p, void *pl) : SqSoulP(b, p, pl)
{
    IniReader ini;
    if (!gSleepRead) {
        gSleepTime = ini.GetFloat("PlayerSleepTime", 15.0f);
        gSleepRead = 1;
    }
    gFoodNuts = ini.GetInt("FoodNutsToFinish", 15);
    if (!gFirstPlayer)
        gFirstPlayer = this;
}
