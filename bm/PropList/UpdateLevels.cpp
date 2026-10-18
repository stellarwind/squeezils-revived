// UpdateLevels, SQZ.EXE 0x42d250.
// Spare declarations preserve VC++ 4.1 register allocation.
void UpdateLevelsPad1();
void UpdateLevelsPad2();
void UpdateLevelsPad3();
void UpdateLevelsPad4();
void UpdateLevelsPad5();
void UpdateLevelsPad6();
void UpdateLevelsPad7();
void UpdateLevelsPad8();
void UpdateLevelsPad9();
void UpdateLevelsPad10();
void UpdateLevelsPad11();
void UpdateLevelsPad12();
void UpdateLevelsPad13();
void UpdateLevelsPad14();
void UpdateLevelsPad15();
#include "PropList/UpdatingProps.h"

struct Prop {
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual void s3(); virtual void s4(); virtual void s5();
    virtual void s6(); virtual void s7();
    virtual void UpdateLevel(); // slot 8
};

void UpdateLevels()
{
    FlushRemovedProps();
    for (int index = 0; index < gUpdatingProps.count; ++index) {
        int queued = 0;
        while (queued < gRemovedPropIndices.count && gRemovedPropIndices.data[queued] != index)
            ++queued;
        if (queued == gRemovedPropIndices.count)
            gUpdatingProps.data[index]->UpdateLevel();
    }
    FlushRemovedProps();
}
