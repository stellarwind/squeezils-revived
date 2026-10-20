// 0x41bbf0: sample each part track into the corresponding pose.
#include "String/String.h"
#define MORPH_CYCLE_SAMPLE_INTERFACE
#include "MorphCycle/MorphCycle.h"

struct PartPose;
struct SamplingPartCycle {
    int Sample(PartPose *pose, float time, int flag); // 0x419960
};
template <class T> struct SampleValueArray {
    void *vtable;
    int count, capacity;
    T **data;
};
struct CyclePartTable {
    int pad0, pad4, count;
    char pad[0x0c];
    SampleValueArray<SamplingPartCycle> values;
    String KeyAt(int index);                         // 0x41bb80
};
struct PoseTable {
    int pad0, pad4, count;
    char pad[0x0c];
    SampleValueArray<PartPose> values;
    String KeyAt(int index);                         // 0x41d800
};

int MorphCycle::SamplePose(PoseTable *pose, float time, int flag)
{
    int result;
    for (int i = 0; i < ((CyclePartTable *)parts)->count; i++) {
        String poseKey = pose->KeyAt(i);
        String cycleKey = ((CyclePartTable *)parts)->KeyAt(i);
        result = ((CyclePartTable *)parts)->values.data[i]->Sample(
            pose->values.data[i], startTime + time, flag);
    }
    return result;
}
