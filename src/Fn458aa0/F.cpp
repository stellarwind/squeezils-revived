// 0x458aa0: SqueezilBody::AnimateOnGround - clip speed follows body speed; the tilt eases between the clips' tilts while blending.
struct Mat { float m[16]; Mat(float angle); ~Mat(); };                 // 0x47dd20 (thiscall, float) / 0x402310
struct Frame;
struct Character {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void Advance(float seconds, int loop);                     // +0x14
    virtual void v6(); virtual void v7(); virtual void v8();
    virtual void SetRoot(Frame *frame, Mat *m);                        // +0x24
    virtual float GetBlend();                                          // +0x28
};
extern float gTimeScale, gFlyTilt, gDegToRad;                          // 0x4c7b84 0x4c7b90 0x49fe38
extern int kTilted[][7];                                               // 0x49f870 (stride 0x1c)
struct Body {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual float Speed();                                             // +0x14
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
    virtual void v18(); virtual void v19(); virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24();
    virtual float Facing();                                            // +0x64
    char pad[0x24 - 4];
    float mx, my, mz;           // 0x24
    char pad1[0x44 - 0x30];
    float fx, fy, fz;           // 0x44
    char pad2[0x220 - 0x50];
    Frame *frame;               // 0x220
    char pad3[0x22c - 0x224];
    Character *character;       // 0x22c
    int mode;                   // 0x230
    int prevMode;               // 0x234
    void AnimateOnGround(float seconds);
};
void Body::AnimateOnGround(float seconds)
{
    float timeScale = gTimeScale;
    if (mode == 2 || mode == 4 || mode == 3) {
        if (Facing() > 0.0f && Speed() > 0.01f) {
            float rate = Speed() / Facing();
            if (fz * mz + fy * my + mx * fx < 0.0f)
                rate = -rate;
            timeScale /= rate;
        }
    }
    character->Advance(seconds / timeScale, 1);
    float blend = character->GetBlend();
    if (blend == -1.0f) {
        float tilt = kTilted[mode][0] ? gFlyTilt : 0.0f;
        Mat m1(tilt * gDegToRad);
        character->SetRoot(frame, &m1);
    } else if (kTilted[mode][0] != kTilted[prevMode][0]) {
        float before = kTilted[prevMode][0] ? gFlyTilt : 0.0f;
        float after = kTilted[mode][0] ? gFlyTilt : 0.0f;
        Mat m2(((after - before) * blend + before) * gDegToRad);
        character->SetRoot(frame, &m2);
    }
}
