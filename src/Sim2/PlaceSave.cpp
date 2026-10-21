#include "Runtime/DynamicCast.h"
// PlaceSimulation slot 11 (SQZ.EXE 0x439db0): write the signature, region/location and the player's inventory to a saved game.
#include <stdio.h>
struct Inventory { int Save(void *file); };              // 0x416600 (ret 4)
struct Possessions { Inventory *Get(); };                // 0x448990
struct PlayerSoul { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
                    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
                    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
                    virtual Possessions *GetPossessions(); };   // slot 18 (+0x48)
struct SoulHolder { void *GetSoul(int id); };            // 0x414080
SoulHolder *GetSimulation();                             // 0x420690 (cdecl)
extern char kSoulType[], kPlayerSoulType[];
extern char *g_signature;                                // 0x4bc6b0
struct PlaceSimulation {
    char pad[0x24];
    int region;                                          // 0x24
    int pad2;
    int location;                                        // 0x2c
    int Save(const char *path);                          // slot 11
};

int PlaceSimulation::Save(const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return 0;
    if (fprintf(f, "%s", g_signature) < 0) {
        fclose(f);
        return 0;
    }
    if (fprintf(f, "%d %d\n", region, location) < 0) {
        fclose(f);
        return 0;
    }
    PlayerSoul *soul = (PlayerSoul *)SqzRTDynamicCast(GetSimulation()->GetSoul(0), 0, kSoulType, kPlayerSoulType, 0);
    Possessions *p = soul->GetPossessions();
    if (!p->Get()->Save(f)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}
