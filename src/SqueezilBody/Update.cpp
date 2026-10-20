// SQZ.EXE 0x457bc0: blend ground-orientation changes and advance animation.
#include "compat41.h"

struct BodyUpdateMatrix {
    float m[16];
    BodyUpdateMatrix() {}
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

void UpdatingSqueezilBody::Update()
{
    BodyUpdateMatrix shown;
    if (IsOnGround()) {
        BodyUpdateMatrix *current;
        if (!noBlend && ((current = GetMatrix())->m[4] != lastMatrix.m[4] ||
                        current->m[5] != lastMatrix.m[5] ||
                        current->m[6] != lastMatrix.m[6])) {
            blendTime = 0.0f;
            blendFrom.CopyInline(lastMatrix);
            blending = 1;
        } else
            unknown20c = 0;
    }
    if (blending) {
        blendTime += TheClock()->SimRate() * kBodyBlendTimeScale;
        if (blendTime >= gBlendSeconds) {
            blending = 0;
            shownMatrix.CopyInline(blendTo);
        } else {
            blendTo.CopyInline(*GetMatrix());
            shown = BlendedMatrix();
            shownMatrix = shown;
        }
    } else {
        shownMatrix = *GetMatrix();
        if (noBlend == 1)
            noBlend = 0;
    }
    lastMatrix = *GetMatrix();
    if (mode == 0x31)
        return;
    float seconds = TheClock()->SimRate() * kBodyClipTimeScale;
    if (IsOnGround())
        AnimateOnGround(seconds);
    else
        AnimateInAir(seconds);
}
