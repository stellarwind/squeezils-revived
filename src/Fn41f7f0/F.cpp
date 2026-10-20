// SqueezilPlayerSoul::FinishPlace (SQZ.EXE 0x41f7f0): leave the place. In replay mode just restart it; otherwise play the
// level-end sound, bank the food nuts, count the medal and the completed place once, and fly back to the region map.
#include "String/String.h"

struct V3 { float x, y, z; V3(const V3 &o); ~V3(); };      // copy 0x4183c0, dtor 0x401660
void PlayEffectAt(String name, V3 position);               // 0x44a250 (cdecl)
void StopLoopingSounds();                                   // 0x44a8f0 (cdecl)
extern int g_loopingSound;                                  // 0x4bd960

struct MatrixP { V3 *Position(); };                         // 0x41faf0
struct BodyP {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual MatrixP *GetMatrix();                           // slot 3 (+0xc)
};
struct InventoryP {
    void Add(const char *goodie, int value);                // 0x416110
    int Count(const char *goodie);                          // 0x416300
    void Clear();                                           // 0x4164a0
};
struct PossessionsP { InventoryP *GetInventory(); };        // 0x448990
struct LandP {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10();
    virtual void Return(int id, PossessionsP *p);           // slot 11 (+0x2c)
    int Completed(int region, int place);                   // 0x41e9b0
};
LandP *GetLand();                                           // 0x420680
struct Sim { virtual void Destroy(int flags); virtual void Init(); virtual void Clear(); };   // slots 0..2
Sim *GetSimulation();                                       // 0x420690
void SetSimulation(Sim *s);                                 // 0x420670 (cdecl)
struct NextRegion : Sim { char pad[0x64]; NextRegion(int region, int place); virtual ~NextRegion(); };   // 0x416e50
struct PlaceSim {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6();
    virtual void Close();                                   // slot 7 (+0x1c)
    char pad[0x24 - 4];
    int region;                                             // 0x24
    int unknown28;
    int place;                                              // 0x2c
    int medal;                                              // 0x30: 0 gold, 1 diamond
    void Start();                                           // 0x438ee0
};

struct SqueezilPlayerSoulF {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16();
    virtual BodyP *GetBody();                               // slot 17 (+0x44)
    virtual PossessionsP *GetPossessions();                 // slot 18 (+0x48)
    virtual PossessionsP *TakePossessions();                // slot 19 (+0x4c)
    int unknown04;
    int replay;                                             // 0x08
    char pad[0x70 - 0x0c];
    PlaceSim *sim;                                          // 0x70
    void FinishPlace();
};

void SqueezilPlayerSoulF::FinishPlace()
{
    if (replay) {
        GetLand()->Return(0, TakePossessions());
        PlaceSim *s = sim;
        s->Close();
        s->Start();
        return;
    }
    if (g_loopingSound)
        StopLoopingSounds();
    PlayEffectAt(String("levelEndEffect"), *GetBody()->GetMatrix()->Position());
    int nuts = GetPossessions()->GetInventory()->Count("FoodNutGoodie");
    int total = GetPossessions()->GetInventory()->Count("TotalFoodNuts");
    int i;
    for (i = 0; i < nuts; i++)
        GetPossessions()->GetInventory()->Add("TotalFoodNuts", total + i);
    GetPossessions()->GetInventory()->Clear();
    if (!GetLand()->Completed(sim->region, sim->place)) {
        switch (sim->medal) {
        case 0:
            GetPossessions()->GetInventory()->Add("GoldPlacesCompleted",
                GetPossessions()->GetInventory()->Count("GoldPlacesCompleted") + 1);
            break;
        case 1:
            GetPossessions()->GetInventory()->Add("DiamondPlacesCompleted",
                GetPossessions()->GetInventory()->Count("DiamondPlacesCompleted") + 1);
            break;
        }
        GetPossessions()->GetInventory()->Count("PlacesCompleted");
        GetPossessions()->GetInventory()->Add("PlacesCompleted", sim->region);
        GetPossessions()->GetInventory()->Add("PlacesCompleted", sim->place);
    }
    int region = sim->region;
    int place = sim->place;
    GetLand()->Return(0, TakePossessions());
    GetSimulation()->Clear();
    Sim *current = GetSimulation();
    if (current)
        current->Destroy(1);
    Sim *next = new NextRegion(region, place);
    SetSimulation(next);
    next->Init();
}
