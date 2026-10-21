// Nonvirtual execution views of the existing PartCycle implementations.
#pragma once
#include "PartCycle/PartCycle.h"
struct PartPose;
// SIZE 0x3c: no additional fields over PartCycle.
struct PartCycleN : PartCycle {
    int Sample(PartPose *pose, float time, int hint); // 0x419960, owner PartCycle/Sample.cpp
};
// SIZE 0x3c: no additional fields over PartCycle.
struct PartCycleB : PartCycle {
    int Blend(PartPose *pose, float time, float weight, int hint); // 0x41a5d0, owner PartCycle/Blend.cpp
};
