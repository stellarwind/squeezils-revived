// PoseTable getters (SQZ.EXE 0x41b6d0 / 0x41b750).
#include "PoseTable/PoseTable.h"

int PoseTable::Count() const
{
    return keys.count;
}

PartPose *PoseTable::At(int index)
{
    return values.data[index];
}
