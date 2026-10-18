// TurnTargetActivity::StepGround (0x401700): turn towards the target point along the surface, then let MoveActivity walk.
#include "compat41.h"
#include <math.h>
struct V3 {
    float x, y, z;
    V3() {}
    V3(const V3 &o) : x(o.x), y(o.y), z(o.z) {}
    void operator=(const V3 &o) { x = o.x; y = o.y; z = o.z; }
    ~V3();                                              // 0x401660
    V3 operator-(const V3 &o) const;                    // 0x401ae0
    V3 operator*(float s) const;                        // 0x401b60
    float Normalize();                                  // 0x47d630
};
static inline float Dot(const V3 &a, const V3 &b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
struct Mi16 { int m[16]; ~Mi16(); Mi16 &operator=(const Mi16 &o); };   // 0x402310, 0x402340
struct Contact {
    int face, second;
    Mi16 mat;
    Contact() : face(-1), second(0) {}
    ~Contact();                                         // 0x4031e0
    Contact &operator=(const Contact &o) { face = o.face; second = o.second; mat = o.mat; return *this; }
};
struct Face { float head[3]; V3 normal; float rest[4]; };
struct Surfaces { char pad[0xc]; Face *faces; };
struct World { Surfaces *SurfacesOf(int face); };        // 0x4365a0
World *GetWorld();                                      // 0x4206a0
struct TBody {
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual float *GetMatrix();                          // slot 3
    virtual void s4(); virtual void s5(); virtual void s6();
    virtual Contact *GetContact();                       // slot 7
};
struct TSoul {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5(); virtual void s6();
    virtual void s7(); virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13();
    virtual void s14(); virtual void s15(); virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19(); virtual void s20();
    virtual void s21(); virtual void s22(); virtual void s23(); virtual void s24(); virtual void s25(); virtual void s26(); virtual void s27();
    virtual void Turn(float angle);                      // slot 28
};
struct GroundControl;
struct TT {
    int vtbl;
    TBody *body;
    TSoul *soul;
    char pad[0x2b0 - 0xc];
    int turn;
    float turnAmount;
    char pad2[4];
    V3 target;                                           // 0x2bc
    float cosAngle;                                      // 0x2c8
    int MoveStep(GroundControl *out);                    // 0x401430
    int StepGround(GroundControl *out);
};
extern float gSlowingCutoff, gSlowingFactor, gMinSlowing;   // 0x4c63c0..c8

int TT::StepGround(GroundControl *out)
{
    V3 along, toTarget, spare, forward, normal, scaled;
    Contact contact;
    toTarget = target - *(V3 *)(body->GetMatrix() + 12);
    toTarget.Normalize();
    contact = *body->GetContact();
    normal = GetWorld()->SurfacesOf(contact.face)->faces[contact.second].normal;
    scaled = normal * Dot(normal, toTarget);
    along = toTarget - scaled;
    along.Normalize();
    forward = *(V3 *)(body->GetMatrix() + 8);
    float cosine = Dot(forward, along);
    float off = 1.0f - cosine;
    cosAngle = cosine;
    if (fabs(off) < 0.0001) {
        turn = 0;
        turnAmount = 0.0f;
        if (cosine != 1.0f) {
            float angle;
            if (cosine <= 1.0f)
                angle = (float)acos(cosine);
            else
                angle = -(float)acos(1.0f - (cosine - 1.0f));
            soul->Turn(angle);
        }
    } else {
        V3 side = *(V3 *)body->GetMatrix();
        if (Dot(side, along) < 0.0f)
            turn = -1;
        else
            turn = 1;
        if (gSlowingCutoff * 0.5f > off)
            turnAmount = off * off * gSlowingFactor + gMinSlowing;
        else if (off < gSlowingCutoff)
            turnAmount = off * gSlowingFactor * 2.0f + gMinSlowing;
        else
            turnAmount = 1.0f;
    }
    return MoveStep(out);
}
