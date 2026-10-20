// PoseTable getters (SQZ.EXE 0x41b6d0 / 0x41b750).
#include "PoseTable/PoseTable.h"

int PoseTable::Count() const
{
    return table.keys.count;
}

PartPose *PoseTable::At(int index)
{
    return table.values.data[index];
}
