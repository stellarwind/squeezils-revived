// PoseTable: the current pose of a whole body (SQZ.EXE 0x41b3f0-0x41b760), VC++ 4.1 dialect.
// Base is Table<String, PartPose*, Array<PartPose*>, 0> (PoseTableBase). Only the two trivial getters are ported.
#pragma once
#include "Container/ContainerTable.h"
#include "String/String.h"

struct PartPose;

typedef Table<String, PartPose *, Array<PartPose *>, 0> PoseTableBase;

struct PoseTable : PoseTableBase {
    PartPose *At(int index);    // 0x41b750
    int Count();                // 0x41b6d0
};
