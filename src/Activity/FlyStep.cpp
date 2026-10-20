// FlyTargetActivity::StepAir (0x401c60): climb or dive to the height of the target and turn to it; the control is filled in by 0x401c30.
#include "compat41.h"
struct V3 {
    float x, y, z;
    V3() {}
    V3(const V3 &o) : x(o.x), y(o.y), z(o.z) {}
    void operator=(const V3 &o) { x = o.x; y = o.y; z = o.z; }
    ~V3();                                              // 0x401660
    V3 operator-(const V3 &o) const;                    // 0x401ae0
    V3 operator*(float s) const;                        // 0x401b60
    float Length() const;                               // 0x47d5f0
    float Normalize();                                  // 0x47d630
};
static inline float Dot(const V3 &a, const V3 &b) { return a.x * b.x + (a.y * b.y + a.z * b.z); }
struct TBody {
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual float *GetMatrix();                          // slot 3
};
struct AirControl;
struct FT {
    int vtbl;
    TBody *body;
    char pad[0x2ac - 8];
    int pitch;
    int turn;
    V3 target;                                           // 0x2b4
    int Fill(AirControl *out);                           // 0x401c30
    int StepAir(AirControl *out);
};

int FT::StepAir(AirControl *out)
{
    V3 toTarget, flat;
    if (body->GetMatrix()[13] - target.y > 0.0f)
        pitch = 1;
    else
        pitch = -1;
    toTarget = target - *(V3 *)(body->GetMatrix() + 12);
    if (toTarget.Length() < 1.0f) {
        turn = 0;
    } else {
        float *first = body->GetMatrix();
        float *second = body->GetMatrix();
        flat = toTarget - *(V3 *)(second + 4) * Dot(*(V3 *)(first + 4), toTarget);
        flat.Normalize();
        float side = Dot(*(V3 *)body->GetMatrix(), flat) < 0.0f ? -1.0f : 1.0f;
        turn = (int)side;
    }
    return Fill(out);
}
