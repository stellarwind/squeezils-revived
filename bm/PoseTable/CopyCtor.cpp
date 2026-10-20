// PoseTable copy constructor (0x41b440): make a deep copy of every part pose.
#define ARRAY_COPY_CTOR
#define V3_COPYCTOR
#include "PartPose/PartPose.h"
#include "PoseTable/PoseTable.h"
PoseTable::PoseTable(const PoseTable &source)
{
    for (int i = 0; i < source.Count(); ++i) {
        PartPose *part = new PartPose(*source.table.values.data[i]);
        table.Insert(source.KeyAt(i), part, -1);
    }
}
