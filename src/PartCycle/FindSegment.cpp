// PartCycle::FindSegment (SQZ.EXE 0x41b2a0, ret 8): which sample interval holds `time`; -1 when it is before the first or after the last.
struct MorphCycleF {
    int SampleCount();              // 0x41ba30
    float TimeAt(int sample);       // 0x41ba50
};
struct PartCycleF {
    char pad[0x34];
    MorphCycleF *cycle;             // 0x34
    int FindSegment(float time, int *hint);
};

int PartCycleF::FindSegment(float time, int *hint)
{
    if (!(cycle->TimeAt(0) < time)) {
        *hint = 0;
        return -1;
    }
    if (cycle->TimeAt(cycle->SampleCount() - 1) <= time) {
        *hint = cycle->SampleCount() - 1;
        return -1;
    }
    if (*hint == -1) {
        for (int i = 0; i <= cycle->SampleCount() - 1; i++) {
            if (!(cycle->TimeAt(i) <= time)) {
                *hint = i;
            (*hint)--;
            return *hint;
            }
        }
        return *hint;
    }
    if (cycle->TimeAt(*hint) <= time) {
        while (cycle->TimeAt(*hint + 1) <= time)
            (*hint)++;
        return *hint;
    }
    for (int i = 0; i <= cycle->SampleCount() - 1; i++) {
        if (!(cycle->TimeAt(i) <= time)) {
            *hint = i;
            (*hint)--;
            return *hint;
        }
    }
    return *hint;
}
