// 0x41ba70: build a pose table by sampling all part tracks at the clip start.
#include "PoseTable/PoseTable.h"
#include "Container/StringTableKey.h"
#include "MorphCycle/MorphCycle.h"

struct PosePartTrack {
    PartPose *NewPose(float time, int hint); // 0x41a3f0
};
// SIZE 0x28: count at +8, pointer values data at +0x24.
struct PosePartTracks {
    int pad0, pad4, count; // 0x00/0x04/0x08
    char pad[0x18]; // 0x0c
    PosePartTrack **values; // 0x24
};
typedef char PosePartTracksSize[(sizeof(PosePartTracks) == 0x28) ? 1 : -1];

PoseTable *MorphCycle::NewPoseTable()
{
    PosePartTracks *tracks = (PosePartTracks *)parts;
    PoseTable *pose = new PoseTable;
    for (int i = 0; i < tracks->count; i++) {
        PartPose *part = tracks->values[i]->NewPose(startTime, -1);
        pose->table.Insert(AnimationTableKeyAt(tracks, i), part, -1);
    }
    return pose;
}
