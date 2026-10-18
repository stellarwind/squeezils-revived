// MorphCycle::BlendPose (SQZ.EXE 0x41bcf0, ret 0x10): blend every part's pose part of the way into the clip.
struct PoseP;
struct PartCycleB { int Blend(PoseP *pose, float time, float weight, int hint); };   // 0x41a5d0
struct PartTable {
    char pad[8];
    int count;                      // 0x08
    char pad2[0x18];
    PartCycleB **values;            // 0x24
};
struct PoseTableB {
    char pad[0x24];
    PoseP **values;                 // 0x24
};
struct MorphCycleB {
    char pad[8];
    float startTime;                // 0x08
    char pad2[0x18];
    PartTable *parts;               // 0x24
    int BlendPose(PoseTableB *pose, float time, float weight, int hint);
};

int MorphCycleB::BlendPose(PoseTableB *pose, float time, float weight, int hint)
{
    int result = hint;
    for (int i = 0; i < parts->count; i++)
        result = parts->values[i]->Blend(pose->values[i], startTime + time, weight, hint);
    return result;
}
