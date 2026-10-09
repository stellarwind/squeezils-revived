// PoseTable: the current pose of a whole body (SQZ.EXE 0x41b3f0-0x41b760), VC++ 4.1 dialect.
// Base is Table<String, PartPose*, Array<PartPose*>, 0> (PoseTableBase). Count/At are the trivial getters;
// ctor/dtor/Find/Assign live in PoseTableLife.cpp.
#pragma once
#include "Container/ContainerTable.h"
#include "String/String.h"

struct PartPose;

typedef Table<String, PartPose *, Array<PartPose *>, 0> PoseTableBase;

struct PoseTable : PoseTableBase {
    PoseTable();                             // 0x41b3f0
    ~PoseTable();                            // 0x41b5f0
    PartPose *Find(const char *partName);    // 0x41b6e0
    PartPose *At(int index);                 // 0x41b750
    int Count() const;                       // 0x41b6d0
    void Assign(const PoseTable &o);         // 0x41b760
};
