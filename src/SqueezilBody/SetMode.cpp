// SqueezilBody::SetMode (SQZ.EXE 0x458820): swap the character's clip; leaving MODE_NONE builds its frames, entering it destroys them.
struct ModeMat {                                         // 64-byte matrix, empty out-of-line destructor 0x402310
    float m[16];
    ModeMat() {}
    ~ModeMat();
    ModeMat &TurnY(float angle);                         // 0x47dd20
};
struct ModeChar {
    virtual void v0();
    virtual void Build(void *frame);                     // +0x04
    virtual void DestroyFrames(void *frame);             // +0x08
    virtual void SetCycle(int clip, int blend);          // +0x0c
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
    virtual void SetRootTransform(void *frame, ModeMat *m);   // +0x24
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13();
    virtual void SetTime(float t);                       // +0x38
};
struct ClipInfo { int tilted; int rest[6]; };            // 28-byte records at 0x49f870
extern int gClips[];                                     // 0x4bf070
extern ClipInfo kClips[];                                // 0x49f870
extern float gNonFlyMorphRotation;                       // 0x4c7b90
extern const float kRadiansPerDegree;                    // 0x49fe38

struct ModeBody {
    char pad[0x220];
    void *frame;                                         // 0x220
    int pad224, pad228;
    ModeChar *character;                                 // 0x22c
    int mode;                                            // 0x230
    int previousMode;                                    // 0x234
    void SetMode(int newMode, int blend, float startTime);
};

void ModeBody::SetMode(int newMode, int blend, float startTime)
{
    if (mode == newMode)
        return;
    ModeMat tilted;
    tilted.TurnY(gNonFlyMorphRotation * kRadiansPerDegree);
    ModeMat straight;
    straight.m[0] = 1; straight.m[1] = 0; straight.m[2] = 0; straight.m[3] = 0;
    straight.m[4] = 0; straight.m[5] = 1; straight.m[6] = 0; straight.m[7] = 0;
    straight.m[8] = 0; straight.m[9] = 0; straight.m[10] = 1; straight.m[11] = 0;
    straight.m[12] = 0; straight.m[13] = 0; straight.m[14] = 0; straight.m[15] = 1;
    if (mode == 0x31)
        character->Build(frame);
    if (newMode != 0x31) {
        character->SetCycle(gClips[newMode], blend);
        character->SetTime(startTime);
        if (kClips[newMode].tilted)
            character->SetRootTransform(frame, &tilted);
        else
            character->SetRootTransform(frame, &straight);
    } else {
        character->DestroyFrames(frame);
    }
    previousMode = mode;
    mode = newMode;
}
