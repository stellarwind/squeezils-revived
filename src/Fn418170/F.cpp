// RegionSimulation::SetLocation (SQZ.EXE 0x418170, ret 4): move to a location, refresh every place whose state changed,
// and when the location is a key spot (300-399) or a relic spot (400-499) collect the key or relic once, with a sound.
#include "String/String.h"

struct V3 {
    float x, y, z;
    V3(float ax, float ay, float az) : x(ax), y(ay), z(az) {}
    V3(const V3 &o);                                     // 0x4183c0
    ~V3();                                               // 0x401660
};
void PlayEffectAt(String name, V3 position);             // 0x44a250 (cdecl)

struct Inventory {
    int unknown00;
    int keys[5][3];                                      // 0x04: [region][key]
    char pad40[0x54 - 0x40];
    int relics[6];                                       // 0x54: [region]
    int KeysFound(int region);                           // 0x416580
};
struct Lent { Inventory *GetInventory(); };              // 0x448990
struct Land {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual Lent *Lend(int id);                          // slot 10 (+0x28)
};
Land *GetLand();                                         // 0x420680
struct Panel {
    void Invalidate();                                   // 0x42db00
    void UpdateRegionSprites();                          // 0x42e2a0
};
Panel *GetPanel();                                       // 0x4206c0

struct Place {
    char pad[0x38];
    int state;                                           // 0x38
    int finished;                                        // 0x3c
    int id;                                              // 0x40
};

struct RegionSimulation {
    char pad[0x24];
    int region;                                          // 0x24
    int location;                                        // 0x28
    char pad2[0x54 - 0x2c];
    Place **places;                                      // 0x54 (the place table's value array)

    int PlaceCount();                                    // 0x418110
    void *GetPlace(int id);                              // 0x418140
    int LocationState(Place *place, int id);             // 0x418590
    void ShowPlace(Place *place, int state);             // 0x4183e0
    void SetLocation(int where);
};

void RegionSimulation::SetLocation(int where)
{
    location = where;
    GetPlace(where);
    for (int i = 0; i < PlaceCount(); i++) {
        Place *p = places[i];
        if (!p->finished) {
            int state = LocationState(p, p->id);
            if (p->state != state)
                ShowPlace(p, state);
        }
    }
    Lent *lent = GetLand()->Lend(0);
    if (location >= 300 && location < 400) {
        int key = location % 300 - 1;
        if (lent->GetInventory()->keys[region][key] == 0) {
            V3 nowhere(0.0f, -100000.0f, 0.0f);
            PlayEffectAt(String("keyEffect"), nowhere);
            lent->GetInventory()->keys[region][key] = 1;
            GetPanel()->Invalidate();
        }
    }
    if (location >= 400 && location < 500 && lent->GetInventory()->KeysFound(region) && region != 1) {
        V3 nowhere(0.0f, -100000.0f, 0.0f);
        PlayEffectAt(String("relicEffect"), nowhere);
        lent->GetInventory()->relics[region] = 1;
        GetPanel()->UpdateRegionSprites();
    }
}
