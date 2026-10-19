// 0x41ba70: build a pose table by sampling all part tracks at the clip start.
#include "PoseTable/PoseTable.h"

struct PosePartTrack {
    PartPose *NewPose(float time, int hint); // 0x41a3f0
};
struct PosePartTracks {
    int pad0, pad4, count;
    char pad[0x18];
    PosePartTrack **values;
    String KeyAt(int index);                // 0x41bb80
};
struct MorphCyclePoseSource {
    char pad[8];
    float startTime;
    char pad0c[0x18];
    PosePartTracks *parts;                  // +0x24
    PoseTable *NewPoseTable();
};

PoseTable *MorphCyclePoseSource::NewPoseTable()
{
    PoseTable *pose = new PoseTable;
    for (int i = 0; i < parts->count; i++) {
        PartPose *part = parts->values[i]->NewPose(startTime, -1);
        pose->table.Insert(parts->KeyAt(i), part, -1);
    }
    return pose;
}
