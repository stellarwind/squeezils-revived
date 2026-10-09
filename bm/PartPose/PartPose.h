// PartPose: one body part's placement + morph weights at a moment (SQZ.EXE 0x4193c0-0x4195b0), VC++ 4.1 dialect.
// Layout from Repo/src/Animation.h. WeightList is PrimitiveArray<float,2>.
#pragma once
#include "Vector3f/Vector3f.h"
#include "Container/ContainerSet.h"

typedef PrimitiveArray<float, 2> WeightList;

struct PartPose {
    Vector3f   position;            // 0x00
    Vector3f   direction;           // 0x0c
    Vector3f   up;                  // 0x18
    WeightList weights;             // 0x24

    PartPose(const Vector3f &position, const Vector3f &direction, const Vector3f &up, const WeightList *weights);   // 0x4193c0
    void Assign(const PartPose &o); // 0x419530
};
