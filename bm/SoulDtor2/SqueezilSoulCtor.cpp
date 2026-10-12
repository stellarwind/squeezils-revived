// SqueezilSoul constructor (SQZ.EXE 0x44f1a0, 3 arguments; the first soul made reads the [Physics] settings).
#include "String/String.h"
struct PhysicalSoulBase { PhysicalSoulBase(); virtual ~PhysicalSoulBase(); };      // ctor 0x446e30
struct GrantMap { virtual ~GrantMap(); };                                           // second base (0x49f72c)
struct DirectionHistory { char d[0x38]; DirectionHistory(int n); ~DirectionHistory(); };   // ctor 0x456da0, dtor 0x456e60
struct SurfaceInfo { int type; String typeName; String value; ~SurfaceInfo(); };    // dtor 0x44f590
struct PhysCore { char d[0x2a0]; };                                                 // SqueezilPhysics, copied by value below
struct Carried { virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void Destroy(int freeMem); };
struct SoulBody { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
                  virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
                  virtual void SetOnGround(int on); };                              // slot 16 (+0x40)
extern int gPhysicsRead2;                                                           // 0x4be788
void ReadSqueezilPhysics();                                                         // 0x44f700
extern PhysCore *gDefaultPhysics;                                                   // 0x4c7ad0
extern PhysCore gDefaultBlock;                                                      // 0x4c7828
struct SqSoul : PhysicalSoulBase, GrantMap {
    int finished, spitPhase;                                        // 0x08, 0x0c
    DirectionHistory directions;                                    // 0x10
    int canSlip;                                                    // 0x2c
    float pitchTarget;                                              // 0x30
    int limitLanding;                                               // 0x34
    int stateAtDeath, state, previousState, frozen;                 // 0x38..0x44
    PhysCore *physics;                                              // 0x48
    int unknown4c;                                                  // 0x4c
    float hopStrength, spitStrength;                                // 0x50, 0x54
    int spitRequested;                                              // 0x58
    float leverSpin;                                                // 0x5c
    SurfaceInfo surface;                                            // 0x60
    Carried *possessions;                                           // 0x6c
    void *place;                                                    // 0x70
    SoulBody *body;                                                 // 0x74
    SqSoul(SoulBody *b, Carried *p, void *pl);                      // 0x44f1a0
    virtual ~SqSoul();
};
SqSoul::SqSoul(SoulBody *b, Carried *p, void *pl)
    : directions(4)
{
    finished = 0;
    spitPhase = 0;
    state = 0;
    frozen = 0;
    canSlip = 1;
    pitchTarget = 1000.0f;
    limitLanding = -1;
    physics = new PhysCore;
    unknown4c = 0;
    spitRequested = 0;
    leverSpin = 0.0f;
    possessions = p;
    place = pl;
    body = b;
    if (!gPhysicsRead2) {
        gPhysicsRead2 = 1;
        gDefaultPhysics = &gDefaultBlock;
        ReadSqueezilPhysics();
    }
    *physics = *gDefaultPhysics;
    surface.type = 0;
    surface.typeName = String(" none");
    surface.value = String("none");
    b->SetOnGround(1);
}
SqSoul::~SqSoul() {}
