// PartCycle: one body part's track in a clip (SQZ.EXE 0x419750-0x419880), VC++ 4.1 dialect.
// Layout from Repo/legacy_gcc/Animation.h. WeightList is PrimitiveArray<float,2>.
#pragma once
#include "Vector3f/Vector3f.h"
#include "Container/ContainerSet.h"

typedef PrimitiveArray<float, 2> WeightList;

struct PartOrientation {
    Vector3f direction;
    Vector3f up;
};

struct MorphCycle;

struct PartCycle {
    PrimitiveArray<Vector3f, 4> positions;     // 0x00
    Array<PartOrientation>      orientations;  // 0x14
    Array<WeightList *>         morph;         // 0x24
    MorphCycle *cycle;                         // 0x34
    int         weightCount;                   // 0x38

    PartCycle(int positionCount, const Vector3f *positionData, int orientationCount, const PartOrientation *orientationData,
              int morphCount, WeightList **morphData, MorphCycle *owner);   // 0x419750
    ~PartCycle();                                                           // 0x419880
};
