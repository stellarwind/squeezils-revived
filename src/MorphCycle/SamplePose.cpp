// 0x41bbf0: sample each part track into the corresponding pose.
#include "String/String.h"
#include "Container/StringTableKey.h"
#include "PoseTable/PoseTable.h"
#include "PartCycle/Execution.h"
#define MORPH_CYCLE_SAMPLE_INTERFACE
#include "MorphCycle/MorphCycle.h"

struct PartPose;
template <class T> struct SampleValueArray {
    void *vtable; // 0x00
    int count, capacity; // 0x04/0x08
    T **data; // 0x0c
};
// SIZE 0x28: key Set prefix, then pointer values Array at +0x18.
struct CyclePartTable {
    int pad0, pad4, count; // 0x00/0x04/0x08
    char pad[0x0c]; // 0x0c
    SampleValueArray<PartCycleN> values; // 0x18
};
typedef char CyclePartTableSize[(sizeof(CyclePartTable) == 0x28) ? 1 : -1];
typedef char SamplePoseTableSize[(sizeof(PoseTable) == 0x28) ? 1 : -1];

int MorphCycle::SamplePose(PoseTable *pose, float time, int flag)
{
    int result;
    for (int i = 0; i < ((CyclePartTable *)parts)->count; i++) {
        String poseKey = AnimationTableKeyAt(&pose->table, i);
        String cycleKey = AnimationTableKeyAt(parts, i);
        result = ((CyclePartTable *)parts)->values.data[i]->Sample(
            pose->table.values.data[i], startTime + time, flag);
    }
    return result;
}
