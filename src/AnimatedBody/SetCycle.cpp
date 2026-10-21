// AnimatedBody::SetCycle (SQZ.EXE 0x41de20): start a clip; with blend the current pose fades out over the new clip's period.
#include "MorphCycle/MorphCycle.h"
#include "PoseTable/PoseTable.h"
typedef PoseTable CPose;
struct CClip;
struct CBody {
    char pad[0x2c];
    CClip *cycle;                                        // 0x2c
    float time;                                          // 0x30
    int hint;                                            // 0x34
    int direction;                                       // 0x38
    int loopMode;                                        // 0x3c
    CPose *pose;                                         // 0x40
    CPose *previousPose;                                 // 0x44
    float blendLeft;                                     // 0x48
    void SetCycle(CClip *clip, int blend);
};

void CBody::SetCycle(CClip *clip, int blend)
{
    time = 0.0f;
    hint = 0;
    cycle = clip;
    direction = 1;
    if (pose == 0)
        pose = ((MorphCycle *)clip)->NewPoseTable();
    if (blend) {
        delete previousPose;
        previousPose = new CPose(*pose);
        blendLeft = ((MorphCycle *)clip)->Period();
    } else {
        delete previousPose;
        previousPose = 0;
    }
}
