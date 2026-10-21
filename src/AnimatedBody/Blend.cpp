// AnimatedBody slot 10 (SQZ.EXE 0x41e2a0): how far the blend into the new clip is (0..1), -1 when not blending.
#include "MorphCycle/MorphCycle.h"
struct Clip;                         // 0x41ba10
struct AnimatedBody2 {
    char pad[0x2c];
    Clip *clip;                                          // 0x2c
    char pad2[0x44 - 0x30];
    int blending;                                        // 0x44
    float blendTime;                                     // 0x48
    float GetBlendFraction();
};

float AnimatedBody2::GetBlendFraction()
{
    if (blending)
        return 1.0f - blendTime / ((MorphCycle *)clip)->Period();
    return -1.0f;
}
