// 0x438020: toggle an unlocked region place, with a 400 ms debounce.
#include <windows.h>
#include <mmsystem.h>

struct RegionPlace {
    char pad[0x38];
    unsigned state;
    int locked;
};
struct PlaceRegion {
    RegionPlace *GetPlace(int id);             // 0x418140
    void SetState(RegionPlace *place, int state); // 0x4183e0
};
struct RegionInput {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14();
    virtual int IsActive();                    // slot 15
};
extern DWORD gLastPlaceToggle;                 // 0x4bc5e8
extern DWORD gPlaceToggleTime;                 // 0x4bc5ec

struct RegionController {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual RegionInput *GetInput(); // slot 17
    char pad[0x84];
    PlaceRegion *region;                       // +0x88
    int pad8c, pad90;
    int previousPlace;                         // +0x94
    void TogglePlace(int id);
};

void RegionController::TogglePlace(int id)
{
    if (GetInput()->IsActive() && id) {
        RegionPlace *place = region->GetPlace(id);
        if (!place->locked) {
            gPlaceToggleTime = timeGetTime();
            if (gPlaceToggleTime - gLastPlaceToggle >= 400) {
                switch (place->state) {
                case 0: region->SetState(place, 1); break;
                case 1: region->SetState(place, 0); break;
                case 2: region->SetState(place, 3); break;
                case 3: region->SetState(place, 2); break;
                default: region->SetState(place, 1); break;
                }
                gLastPlaceToggle = gPlaceToggleTime;
            }
        }
    }
    if (previousPlace && id != previousPlace) {
        RegionPlace *place = region->GetPlace(previousPlace);
        if (!place->locked) {
            if (place->state == 2) {
                region->SetState(place, 3);
                return;
            }
            if (place->state == 0)
                region->SetState(place, 1);
        }
    }
}
