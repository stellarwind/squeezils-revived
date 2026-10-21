// 0x41bcf0: blend each clip track into its corresponding part pose.
#include "MorphCycle/MorphCycle.h"
#include "PartCycle/Execution.h"
#include "PoseTable/PoseTable.h"
// SIZE 0x28: String key Set at +4, pointer Array at +0x18.
struct BlendingPartTable {
    char pad[8]; // 0x00
    int count; // 0x08
    char pad0c[0x18]; // 0x0c
    PartCycleB **values; // 0x24
};
int MorphCycle::BlendPose(PoseTable *pose, float time, float weight, int hint)
{
    BlendingPartTable *tracks = (BlendingPartTable *)parts;
    int result = hint;
    for (int i = 0; i < tracks->count; ++i)
        result = tracks->values[i]->Blend(pose->table.values.data[i], startTime + time, weight, hint);
    return result;
}
