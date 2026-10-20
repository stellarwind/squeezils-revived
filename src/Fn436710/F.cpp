// SqueezilRegionSoul constructor (SQZ.EXE 0x436710, ret 0xc): the SqueezilSoul part (0x44f1a0), the input-event base at
// +0x80, the one-time [region flight] tuning globals, then stand on the current place and record it as visited.
#include "IniReader/IniReader.h"

struct SoulBodyR; struct CarriedR;
struct Float3 { float x, y, z; };
struct PhysBaseR { virtual ~PhysBaseR(); };
struct GrantR { virtual void g0(); };
struct SqSoulR : PhysBaseR, GrantR {
    char body[0x74 - 8];
    struct BodyR *body74;                                          // 0x74
    char pad78[0x80 - 0x78];
    SqSoulR(SoulBodyR *b, CarriedR *p, void *pl);                  // 0x44f1a0
    void SetState(int state, int arg);                             // 0x450320
};
struct EventTargetR { virtual void Event(); };                     // vtable 0x49d2c4
struct V3R { float x, y, z; V3R() {} ~V3R(); };                    // 0x401660
struct MatrixR {
    float m[16];
    MatrixR() {
        m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f;
        m[4] = 0.0f; m[5] = 1.0f; m[6] = 0.0f; m[7] = 0.0f;
        m[8] = 0.0f; m[9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
        m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
    }
    ~MatrixR();                                                    // 0x402310
    MatrixR &SetTranslation(const Float3 &t);                      // 0x47e330
};
struct BodyR {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
    virtual void SetMatrix(MatrixR *m);                            // slot 9 (+0x24)
};
struct RegionSimR {
    Float3 *GetPlace(int id);                                      // 0x418140
    int GetLocation();                                             // 0x418150
    int GetRegion();                                               // 0x418160
};
struct InventoryR { void Add(const char *goodie, int value); };    // 0x416110
struct LentR { InventoryR *GetInventory(); };                      // 0x448990
struct LandR {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual LentR *Lend(int id);                                   // slot 10 (+0x28)
};
LandR *GetLand();                                                  // 0x420680

extern int gRegionTuningRead;                                      // 0x4bc5e4
extern int gDisplayRegionBridge;                                   // 0x4c7334
extern int gRegionFlightDuration;                                  // 0x4c7328
extern float gTurnRateScale;                                       // 0x4c733c
extern float gTurnRateDamping;                                     // 0x4c7330
extern float gRegionFlightArcHeightRatio;                          // 0x4c7344
extern float gRegionFlightArcInitialTilt;                          // 0x4c732c
extern float gRegionBridgeWidth;                                   // 0x4c7338

struct SqueezilRegionSoulC : SqSoulR, EventTargetR {
    int unknown84;                                                 // 0x84
    RegionSimR *region;                                            // 0x88
    int moving;                                                    // 0x8c
    int location;                                                  // 0x90
    int target;                                                    // 0x94
    V3R from, to;                                                  // 0x98 0xa4
    int unknownB0, unknownB4;                                      // 0xb0
    char padB8[0xe4 - 0xb8];
    int pending;                                                   // 0xe4
    SqueezilRegionSoulC(SoulBodyR *b, CarriedR *p, RegionSimR *sim);
    virtual ~SqueezilRegionSoulC();
    int BeenTo(int place);                                         // 0x437f40
};

SqueezilRegionSoulC::SqueezilRegionSoulC(SoulBodyR *b, CarriedR *p, RegionSimR *sim) : SqSoulR(b, p, 0)
{
    region = sim;
    if (!gRegionTuningRead) {
        IniReader ini;
        gDisplayRegionBridge = ini.GetInt("DisplayRegionBridge", 0);
        gRegionFlightDuration = ini.GetInt("RegionFlightDuration", 30);
        gTurnRateScale = ini.GetFloat("TurnRateScale", 0.02f);
        gTurnRateDamping = ini.GetFloat("TurnRateDamping", 0.1f);
        gRegionFlightArcHeightRatio = ini.GetFloat("RegionFlightArcHeightRatio", 0.1f);
        gRegionFlightArcInitialTilt = ini.GetFloat("RegionFlightArcInitialTilt", 0.3f);
        gRegionBridgeWidth = ini.GetFloat("RegionBridgeWidth", 0.05f);
        gRegionTuningRead = 1;
    }
    location = region->GetLocation();
    target = 0;
    MatrixR m;
    m.SetTranslation(*region->GetPlace(location));
    body74->SetMatrix(&m);
    moving = 0;
    SetState(0, 0);
    unknownB4 = 0;
    unknownB0 = 0;
    pending = -1;
    unknown84 = 0;
    if (!BeenTo(location)) {
        InventoryR *inv = GetLand()->Lend(0)->GetInventory();
        inv->Add("PlacesBeenTo", region->GetRegion());
        inv->Add("PlacesBeenTo", region->GetLocation());
    }
}
