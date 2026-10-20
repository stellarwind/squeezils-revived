#pragma once
#include "compat41.h"

struct BodyBlendVector;

struct BodyUpdateMatrix {
    float m[16];
    BodyUpdateMatrix() {}
    BodyUpdateMatrix(const BodyUpdateMatrix &other); // 0x4099e0
    void SetRows(const BodyBlendVector &right, const BodyBlendVector &up,
                 const BodyBlendVector &forward, const BodyBlendVector &position); // 0x409f00
    void Orthonormalize(); // 0x47e350
    ~BodyUpdateMatrix(); // 0x402310
    BodyUpdateMatrix &operator=(const BodyUpdateMatrix &other); // 0x402340
    void CopyInline(const BodyUpdateMatrix &other) {
        m[0] = other.m[0]; m[1] = other.m[1]; m[2] = other.m[2]; m[3] = other.m[3];
        m[4] = other.m[4]; m[5] = other.m[5]; m[6] = other.m[6]; m[7] = other.m[7];
        m[8] = other.m[8]; m[9] = other.m[9]; m[10] = other.m[10]; m[11] = other.m[11];
        m[12] = other.m[12]; m[13] = other.m[13]; m[14] = other.m[14]; m[15] = other.m[15];
    }
};
struct BodyUpdateClock { unsigned SimRate(); }; // 0x41ec00
BodyUpdateClock *TheClock(); // 0x4206d0
extern float gBlendSeconds; // 0x4c7b88
extern const float kBodyBlendTimeScale, kBodyClipTimeScale; // 0x49fe30, 0x49fe34

struct UpdatingSqueezilBody {
    virtual void slot0(); virtual void slot1(); virtual void slot2();
    virtual BodyUpdateMatrix *GetMatrix(); // slot 3
    virtual void slot4(); virtual void slot5(); virtual void slot6(); virtual void slot7();
    virtual void slot8(); virtual void slot9(); virtual void slot10(); virtual void slot11();
    virtual void slot12(); virtual void slot13(); virtual void slot14();
    virtual int IsOnGround(); // slot 15, +0x3c
    char pad04[0xc0 - 4];
    BodyUpdateMatrix lastMatrix; // +0xc0
    BodyUpdateMatrix shownMatrix; // +0x100
    BodyUpdateMatrix blendFrom; // +0x140
    BodyUpdateMatrix unused180; // +0x180
    BodyUpdateMatrix blendTo; // +0x1c0
    float blendTime; // +0x200
    int blending, noBlend, unknown20c;
    char pad210[0x230 - 0x210];
    int mode; // +0x230
    BodyUpdateMatrix BlendedMatrix(); // 0x4574b0
    void AnimateOnGround(float seconds); // 0x458aa0
    void AnimateInAir(float seconds); // 0x458cd0
    void Update();
};

