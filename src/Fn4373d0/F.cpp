// SqueezilRegionSoul::Update (SQZ.EXE 0x4373d0, vtable slot): the region map. While standing on a place, pick the
// open destination closest to the facing direction (within 20 degrees), highlight it and start the flight there on a
// forward push; while flying, lerp along an arc from the old place to the new one and land on it after
// [region flight] RegionFlightDuration ticks; otherwise turn on the spot.
#include <math.h>

struct V3 {
    float x, y, z;
    V3() {}
    V3(float ax, float ay, float az) : x(ax), y(ay), z(az) {}
    V3(const V3 &o) : x(o.x), y(o.y), z(o.z) {}
    void operator=(const V3 &o) { x = o.x; y = o.y; z = o.z; }
    ~V3();                                                   // 0x401660
    V3 operator-(const V3 &o) const;                         // 0x401ae0
    V3 operator*(float s) const;                             // 0x401b60
    V3 operator+(const V3 &o) const;                         // 0x403070
    V3 Cross(const V3 &o) const;                             // 0x41a350
    float Length() const;                                    // 0x47d5f0
    float Normalize();                                       // 0x47d630
};
// SIZE 0x40
struct Matrix {
    float m[16];
    Matrix() {}
    Matrix(const V3 &x, const V3 &y, const V3 &z, const V3 &p) {
        m[0] = x.x; m[1] = x.y; m[2] = x.z; m[3] = 0.0f;
        m[4] = y.x; m[5] = y.y; m[6] = y.z; m[7] = 0.0f;
        m[8] = z.x; m[9] = z.y; m[10] = z.z; m[11] = 0.0f;
        m[12] = p.x; m[13] = p.y; m[14] = p.z; m[15] = 1.0f;
    }
    ~Matrix();                                               // 0x402310
    Matrix &TurnY(float angle);                              // 0x47dd20
    Matrix operator*(const Matrix &o) const;                 // 0x47dea0
    void Orthonormalize();                                   // 0x47e350
    V3 &Forward() { return *(V3 *)&m[8]; }
    V3 &Position() { return *(V3 *)&m[12]; }
};

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix *GetMatrix();                             // vtable+0x0c
    virtual void b4(); virtual void b5(); virtual void b6(); virtual void b7(); virtual void b8();
    virtual void SetMatrix(Matrix *m);                       // vtable+0x24
    virtual void b10(); virtual void b11(); virtual void b12(); virtual void b13(); virtual void b14();
    virtual int Grounded();                                  // vtable+0x3c
};
struct Destinations {
    virtual int At(int i);                                   // vtable+0x00
    int count;                                               // 0x04
};
// SIZE 0x44
struct Place {
    V3 pos;                                                  // 0x00
    char pad0c[0x10 - 0x0c];
    Destinations *destinations;                              // 0x10
    char pad14[0x3c - 0x14];
    int kind;                                                // 0x3c
    int id;                                                  // 0x40
};
struct RegionSim {
    Place *GetPlace(int id);                                 // 0x418140
    int GetLocation();                                       // 0x418150
    int GetRegion();                                         // 0x418160
    void Select(int id);                                     // 0x418170
};
struct Inventory { void Add(const char *goodie, int value); };   // 0x416110
struct Lent { Inventory *GetInventory(); };                  // 0x448990
struct Land {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual Lent *Lend(int id);                              // vtable+0x28
    int Completed(int region, int place);                    // 0x41e9b0
};
struct Application {
    float ForwardAxis();                                     // 0x43bc70
    float TurnAxis();                                        // 0x43bd20
};
Land *GetLand();                                             // 0x420680
void *GetWorld();                                            // 0x4206a0
Application *GetApplication();                               // 0x4206b0

extern int gDisplayRegionBridge;                             // 0x4c7334
extern int gRegionFlightDuration;                            // 0x4c7328
extern float gTurnRateScale;                                 // 0x4c733c
extern float gTurnRateDamping;                               // 0x4c7330
extern float gRegionFlightArcHeightRatio;                    // 0x4c7344
extern float gRegionFlightArcInitialTilt;                    // 0x4c732c

// SIZE 0xe8
struct SqueezilRegionSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16();
    virtual SoulBody *Body();                                // vtable+0x44
    virtual void s18(); virtual void s19(); virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23();
    virtual void SetState(int state, int arg);               // vtable+0x60
    char pad04[0x84 - 0x04];
    int flying;                                              // 0x84
    RegionSim *region;                                       // 0x88
    int tick;                                                // 0x8c
    int location;                                            // 0x90
    int target;                                              // 0x94
    V3 from;                                                 // 0x98
    V3 to;                                                   // 0xa4
    float turnRate;                                          // 0xb0
    char padB4[0xe8 - 0xb4];
    void Update();
    void StartFunction();                                    // 0x436a60
    int LinkOpen(Place *from, Place *here);                  // 0x437e40
    int BeenTo(int place);                                   // 0x437f40
    void TogglePlace(int place);                             // 0x438020
    void ShowCursor(int place);                              // 0x438110
};

void SqueezilRegionSoul::Update()
{
    V3 at;
    Matrix turn;
    Matrix m;
    GetWorld();
    int best = 0;
    float bestAngle = 0.34906584f;
    V3 forward = Body()->GetMatrix()->Forward();
    float heading = (float)atan2(forward.x, forward.z);
    V3 position = Body()->GetMatrix()->Position();
    Place *here = region->GetPlace(location);
    int hereDone;
    if (here->kind == 0 && GetLand()->Completed(region->GetRegion(), here->id))
        hereDone = 1;
    else
        hereDone = 0;
    for (int i = 0; i < here->destinations->count; i++) {
        Place *there = region->GetPlace(here->destinations->At(i));
        GetLand()->Lend(0)->GetInventory();
        int thereDone = there->kind == 0 && GetLand()->Completed(region->GetRegion(), there->id);
        if (here->kind == 2 || here->kind == 1 || hereDone || thereDone
            || (there->kind != 2 && BeenTo(there->id))
            || (here->id >= 300 && here->id < 400) || (there->id >= 300 && there->id < 400)
            || LinkOpen(here, there)
            || (here->id >= 400 && here->id < 500 && there->kind != 2)) {
            V3 p = there->pos;
            float angle = (float)fabs(atan2(p.x - position.x, p.z - position.z) - heading);
            while (angle > 6.2831855f)
                angle -= 6.2831855f;
            if (angle < bestAngle) {
                bestAngle = angle;
                best = here->destinations->At(i);
            }
        }
    }
    if (Body()->Grounded() && target != best && gDisplayRegionBridge)
        ShowCursor(best);
    TogglePlace(best);
    target = best;
    if (Body()->Grounded() && target) {
        if (GetApplication()->ForwardAxis() > 0.0) {
            from = region->GetPlace(location)->pos;
            to = region->GetPlace(target)->pos;
            SetState(0x11, 0);
            tick = 0;
        }
    }
    if (!Body()->Grounded()) {
        if (++tick < gRegionFlightDuration) {
            flying = 1;
            V3 span = to - from;
            float t = (float)tick / (float)gRegionFlightDuration;
            V3 step = span * t;
            at = step + from;
            V3 dir = span;
            dir.Normalize();
            float tilt = gRegionFlightArcInitialTilt * -2.0f * t;
            float s = (float)fabs(1.0f - t * 2.0f);
            dir.y = tilt + dir.y + gRegionFlightArcInitialTilt;
            float len = span.Length();
            at.y = -(s * s * len * gRegionFlightArcHeightRatio) + at.y + len * gRegionFlightArcHeightRatio;
            V3 side = V3(0.0f, 1.0f, 0.0f).Cross(dir);
            V3 top = dir.Cross(side);
            Matrix arc(side, top, dir, at);
            arc.Orthonormalize();
            Body()->SetMatrix(&arc);
        } else {
            location = target;
            SetState(0x13, 0);
            SetState(0, 0);
            flying = 0;
            Matrix *bm = Body()->GetMatrix();
            V3 facing(bm->m[8], 0.0f, bm->m[10]);
            facing.Normalize();
            V3 up(0.0f, 1.0f, 0.0f);
            V3 side = up.Cross(facing);
            V3 p = region->GetPlace(location)->pos;
            Matrix land(side, up, facing, p);
            Body()->SetMatrix(&land);
            region->Select(location);
            StartFunction();
            if (!BeenTo(location)) {
                Inventory *inv = GetLand()->Lend(0)->GetInventory();
                inv->Add("PlacesBeenTo", region->GetRegion());
                inv->Add("PlacesBeenTo", region->GetLocation());
            }
        }
    } else {
        turnRate = GetApplication()->TurnAxis() * gTurnRateScale + turnRate;
        turn.TurnY(turnRate * 0.017453292f);
        m = turn * *Body()->GetMatrix();
        Body()->SetMatrix(&m);
        turnRate = turnRate * gTurnRateDamping;
    }
}
