// 0x458cd0: SqueezilBody::AnimateInAir - fly pose from the fly clip, mixed with up/down/left/right clips by pitch and roll.
struct PoseTable;
struct Clip {
    int SamplePose(PoseTable *pose, float time, int flag);                   // 0x41bbf0
    int BlendPose(PoseTable *pose, float time, float weight, int flag);      // 0x41bcf0 (ret 0x10)
};
struct Character {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void Advance(float seconds, int loop);                           // +0x14
    char pad[0x40 - 4];
    PoseTable *pose;                                                         // 0x40
};
extern float gDive, gClimb, gTimeScale, gBank;                               // 0x4c7b7c 0x4c7b80 0x4c7b84 0x4c7b8c
extern Clip *gFly, *gFlyUp, *gFlyDown, *gFlyLeft, *gFlyRight;               // 0x4bf0b0 b4 b8 bc c0 (see targets)
struct Body {
    char pad[0x210];
    float pitch;            // 0x210
    char pad2[4];
    float roll;             // 0x218
    char pad3[0x22c - 0x21c];
    Character *character;   // 0x22c
    int mode;               // 0x230
    void AnimateInAir(float seconds);
};
void Body::AnimateInAir(float seconds)
{
    if (mode == 0x15 || mode == 0x16 || mode == 0x26 || mode == 0x28 || mode == 0x29) {
        character->Advance(seconds / gTimeScale, 1);
        return;
    }
    float p = pitch;
    float r = roll;
    float up, down, left, right;
    up = down = left = right = 0.0f;
    if (p < gClimb)
        p = gClimb;
    if (gDive < p)
        p = gDive;
    if (*(unsigned *)&p > 0x80000000)
        up = p / gClimb;
    else
        down = p / gDive;
    if (r < -gBank)
        r = -gBank;
    if (gBank < r)
        r = gBank;
    if (*(unsigned *)&r > 0x80000000)
        left = -(r / gBank);
    else
        right = r / gBank;
    gFly->SamplePose(character->pose, 0.0f, -1);
    if (*(int *)&up > 0)
        gFlyUp->BlendPose(character->pose, 0.0f, up, -1);
    if (*(int *)&down > 0)
        gFlyDown->BlendPose(character->pose, 0.0f, down, -1);
    if (*(int *)&left > 0)
        gFlyLeft->BlendPose(character->pose, 0.0f, left, -1);
    if (*(int *)&right > 0)
        gFlyRight->BlendPose(character->pose, 0.0f, right, -1);
}
