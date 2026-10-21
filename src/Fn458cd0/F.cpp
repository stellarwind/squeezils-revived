// 0x458cd0: SqueezilBody::AnimateInAir - fly pose from the fly clip, mixed with up/down/left/right clips by pitch and roll.
struct PoseTable;
#include "MorphCycle/MorphCycle.h"
struct Clip;
struct AirAnimationCharacter {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void Advance(float seconds, int loop);                           // +0x14
    char pad[0x40 - 4];
    PoseTable *pose;                                                         // 0x40
};
extern float gDive; // 0x4c7b7c
extern float gClimb; // 0x4c7b80
extern float gTimeScale; // 0x4c7b84
extern float gBank; // 0x4c7b8c
extern Clip *gFly; // 0x4bf0b0
extern Clip *gFlyUp; // 0x4bf0b4
extern Clip *gFlyDown; // 0x4bf0c0
extern Clip *gFlyLeft; // 0x4bf0b8
extern Clip *gFlyRight; // 0x4bf0bc
#include "SqueezilBody/BodyUpdate.h"
void UpdatingSqueezilBody::AnimateInAir(float seconds)
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
    ((MorphCycle *)gFly)->SamplePose(character->pose, 0.0f, -1);
    if (*(int *)&up > 0)
        ((MorphCycle *)gFlyUp)->BlendPose(character->pose, 0.0f, up, -1);
    if (*(int *)&down > 0)
        ((MorphCycle *)gFlyDown)->BlendPose(character->pose, 0.0f, down, -1);
    if (*(int *)&left > 0)
        ((MorphCycle *)gFlyLeft)->BlendPose(character->pose, 0.0f, left, -1);
    if (*(int *)&right > 0)
        ((MorphCycle *)gFlyRight)->BlendPose(character->pose, 0.0f, right, -1);
}
