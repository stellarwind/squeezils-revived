// 0x401fd0: restore a creature's placement and restart its pending behavior.
#include "Activity/Activity.h"

struct RegenPlacement {
    char data[0xb0];
    RegenPlacement(const RegenPlacement &other); // 0x4020b0
    ~RegenPlacement();                         // 0x402250
};
struct RegenPlacements {
    virtual RegenPlacement *Get(int index);
};
struct RegenLevel { char pad[0x2a8]; RegenPlacements creatures; };
struct RegenSoul;
struct RegenPlace {
    int FindIdBySoul(RegenSoul *soul);            // 0x4140b0
    RegenLevel *GetLevel();                      // 0x439b50
};
struct RegenTemplate {
    int TrySetState(int state, RegenSoul *soul);  // 0x4486f0
    void RunPending(RegenSoul *soul);             // 0x4487f0
};
struct RegenSoul {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void Regenerate(); // slot 21
    char pad04[0x70 - 4];
    RegenPlace *place;
    char pad74[0x98 - 0x74];
    int levelIndex;
    char pad9c[0xf4 - 0x9c];
    RegenTemplate *prototype;
};
struct RegenBody {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual AV3 *GetFacing();                     // slot 4
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void SetFacing(AV3 *direction, float value); // slot 10
    void Place(RegenPlacement placement);          // 0x4577e0
};

int RegenActivity::StepGround(AGroundControl *)
{
    ((RegenSoul *)soul)->place->FindIdBySoul((RegenSoul *)soul);
    int index = ((RegenSoul *)soul)->levelIndex;
    ((RegenBody *)body)->Place(*((RegenSoul *)soul)->place->GetLevel()->creatures.Get(
        index));
    ((RegenSoul *)soul)->Regenerate();
    ((RegenBody *)body)->SetFacing(((RegenBody *)body)->GetFacing(), 0.0f);
    ((RegenSoul *)soul)->prototype->TrySetState(0, (RegenSoul *)soul);
    ((RegenSoul *)soul)->prototype->RunPending((RegenSoul *)soul);
    return 0;
}
