// PoseTable lifecycle and lookup (SQZ.EXE 0x41b3f0 / 0x41b5f0 / 0x41b6e0 / 0x41b760).
#include "PartPose/PartPose.h"
#include "PoseTable/PoseTable.h"

PoseTable::PoseTable()
{
}

PoseTable::~PoseTable()
{
    int i = 0;
    for (;;) {
        if (i >= table.keys.count) break;
        delete table.values.data[i];
        ++i;
    }
}

PartPose *PoseTable::Find(const char *partName)
{
    return table.Get(String(partName));
}

void PoseTable::Assign(const PoseTable &o)
{
    for (int i = 0; i < o.Count(); i++)
        table.values.data[i]->Assign(*o.table.values.data[i]);
}
