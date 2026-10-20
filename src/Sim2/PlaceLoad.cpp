// PlaceSimulation slot 10 (SQZ.EXE 0x439b70): load a saved game, hand the inventory to the player and switch to a new RegionSimulation.
#include <stdio.h>
#include <string.h>
struct Inventory { char pad[0x6c]; Inventory(); virtual ~Inventory(); int Load(void *file); };   // ctor 0x415ef0, 0x416820 (ret 4)
struct Possessions { void SetInventory(Inventory *i); };                          // 0x4489a0
struct PlayerSoul { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
                    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
                    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
                    virtual void v18(); virtual Possessions *TakePossessions(); };   // slot 19 (+0x4c)
struct Land { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
              virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10();
              virtual void Return(int id, Possessions *p); };   // slot 11 (+0x2c)
Land *GetLand();                                                                    // 0x420680 (cdecl)
struct Sim { virtual void Destroy(int flags); virtual void Init(); virtual void Clear();   // slots 0..2
             void *GetSoul(int id); };                                              // 0x414080
Sim *GetSimulation();                                                               // 0x420690 (cdecl)
void SetSimulation(Sim *s);                                                         // 0x420670 (cdecl)
struct RegionSimulation : Sim { char pad[0x64]; RegionSimulation(int region, int location); virtual ~RegionSimulation(); };   // 0x416e50 (ret 8)
struct Panel { void UpdateRegionSprites(); };                                       // 0x42e2a0
Panel *GetPanel();                                                                  // 0x4206c0 (cdecl)
void SetGamePaused(int p);                                                          // 0x43ed80 (cdecl)
void *DynamicCast(void *p, int a, void *from, void *to, int b);                     // 0x48f060
extern char kSoulType[], kPlayerSoulType[];
extern char *g_signature;                                                           // 0x4bc6b0
struct PlaceSimulation {
    int Load(const char *path);                                                     // slot 10
};

int PlaceSimulation::Load(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return 0;
    char line[256];
    if (!fgets(line, 255, f)) {
        fclose(f);
        return 0;
    }
    if (strcmp(line, g_signature) != 0) {
        fclose(f);
        return 0;
    }
    if (!fgets(line, 255, f)) {
        fclose(f);
        return 0;
    }
    int region, location;
    sscanf(line, "%d %d", &region, &location);
    Inventory *inventory = new Inventory;
    if (!inventory->Load(f)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    PlayerSoul *soul = (PlayerSoul *)DynamicCast(GetSimulation()->GetSoul(0), 0, kSoulType, kPlayerSoulType, 0);
    Possessions *p = soul->TakePossessions();
    p->SetInventory(inventory);
    GetLand()->Return(0, p);
    GetSimulation()->Clear();
    Sim *current = GetSimulation();
    if (current)
        current->Destroy(1);
    RegionSimulation *next = new RegionSimulation(region, location);
    SetSimulation(next);
    next->Init();
    GetPanel()->UpdateRegionSprites();
    SetGamePaused(1);
    return 1;
}
