// SQZ.EXE 0x436af0: Enter on the input-event base at soul + 0x80.
#include <string.h>
#include "String/String.h"

struct EnterVector {
    float x, y, z;
    EnterVector(float a, float b, float c) : x(a), y(b), z(c) {}
    EnterVector(const EnterVector &v);                 // 0x4183c0
    ~EnterVector();                                   // 0x401660
};
void PlayEffectAt(String name, EnterVector position);  // 0x44a250
struct EnterDestinations {
    virtual int At(int i);
    int count;                                        // +4
};
struct EnterPlace { char pad[0xc]; EnterDestinations *destinations; char pad2[0x2c]; int kind; };
struct EnterInventory {
    char pad[0x54]; int relic[8];
    int KeysFound(int region);                        // 0x416580
    int AllRelics();                                  // 0x4165b0
};
struct EnterOwner { EnterInventory *GetInventory(); }; // 0x448990
struct EnterLand {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual EnterOwner *Lend(int id);                  // +0x28
    virtual void Return(int id, EnterOwner *p);        // +0x2c
};
EnterLand *GetLand();                                 // 0x420680
struct EnterSim {
    virtual void Destroy(int flags);
    virtual void Init();
    virtual void Clear();
    virtual void v3(); virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10();
    virtual int Save(const char *path);                // +0x2c
};
EnterSim *GetSimulation();                            // 0x420690
void SetSimulation(EnterSim *s);                      // 0x420670
struct EnterRegionSim : EnterSim { char pad[0x64]; EnterRegionSim(int region, int location); }; // 0x416e50
struct EnterPlaceSim : EnterSim { char pad[0x40]; EnterPlaceSim(int region, int place, int location); }; // 0x438850
struct EnterRegion {
    int GetLocation();                                // 0x418150
    int GetRegion();                                  // 0x418160
    EnterPlace *GetPlace(int id);                      // 0x418140
};
struct EnterPrimary {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18();
    virtual EnterOwner *TakePossessions();             // +0x4c
};
extern char g_saveDir[];                              // 0x4c6400
struct RegionSoulInput {
    int vtable;
    int flying;                                       // +4 (primary +0x84)
    EnterRegion *region;                              // +8
    void Enter();
};

void RegionSoulInput::Enter()
{
    if (flying)
        return;
    EnterRegion *r = region;
    EnterPlace *here = r->GetPlace(r->GetLocation());
    EnterDestinations *destinations = here->destinations;
    if (!destinations->count)
        return;
    if (here->kind == 2) {
        EnterInventory *inventory = GetLand()->Lend(0)->GetInventory();
        if (region->GetRegion() != 1) {
            if (region->GetRegion() == 6 && destinations->At(0) == 7) {
                if (!inventory->KeysFound(region->GetRegion())) return;
            } else {
                if (region->GetRegion() == 7) return;
                if (!inventory->relic[region->GetRegion()]) return;
            }
        } else {
            int to = destinations->At(0);
            if (to == 6) {
                if (!inventory->AllRelics()) return;
            } else if (inventory->relic[to]) return;
        }
    }
    EnterVector faraway(0.0f, -100000.0f, 0.0f);
    PlayEffectAt(String("enterEffect"), faraway);
    EnterSim *next;
    if (destinations->count > 1) {
        GetLand()->Return(0, ((EnterPrimary *)((char *)this - 0x80))->TakePossessions());
        int nextRegion = destinations->At(0);
        int nextLocation = destinations->At(1);
        GetSimulation()->Clear();
        EnterSim *old = GetSimulation();
        if (old) old->Destroy(1);
        next = new EnterRegionSim(nextRegion, nextLocation);
    } else {
        char path[80];
        strcpy(path, g_saveDir);
        strcat(path, "Autosave");
        GetSimulation()->Save(path);
        GetLand()->Return(0, ((EnterPrimary *)((char *)this - 0x80))->TakePossessions());
        int nextRegion = region->GetRegion();
        int nextPlace = destinations->At(0);
        int nextLocation = region->GetLocation();
        GetSimulation()->Clear();
        EnterSim *old = GetSimulation();
        if (old) old->Destroy(1);
        next = new EnterPlaceSim(nextRegion, nextPlace, nextLocation);
    }
    SetSimulation(next);
    next->Init();
}
