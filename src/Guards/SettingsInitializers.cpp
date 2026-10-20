// 0x44f0a0 / 0x4594d0: construct the empty vector pair in each static settings block.
#include "compat41.h"

struct Float3 {
    float x, y, z;
    Float3() {}
    ~Float3();                                 // 0x401660
};
struct Pair {
    Float3 a, b;
    Pair() {}
    ~Pair();                                   // 0x4052b0
};
struct SqueezilSettingsInit : Pair {
    char remaining[0x2a0 - 24];
    ~SqueezilSettingsInit() {}
};
struct GoodieSettingsInit : Pair {
    char remaining[0x40 - 24];
    ~GoodieSettingsInit() {}
};
extern SqueezilSettingsInit gSqueezilSettingsInit; // 0x4c7828
extern GoodieSettingsInit gGoodieSettingsInit;    // 0x4c7bc0

void Thunk44f0a0()
{
    // VC++ 4.1 permits direct constructor calls on existing static storage.
    gSqueezilSettingsInit.SqueezilSettingsInit::SqueezilSettingsInit();
}

void Thunk4594d0()
{
    gGoodieSettingsInit.GoodieSettingsInit::GoodieSettingsInit();
}
