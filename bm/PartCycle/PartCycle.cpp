// PartCycle ctor (0x419750) and dtor (0x419880).
#include "PartCycle/PartCycle.h"

PartCycle::PartCycle(int positionCount, const Vector3f *positionData, int orientationCount, const PartOrientation *orientationData,
                     int morphCount, WeightList **morphData, MorphCycle *owner)
    : positions(positionCount, positionData),
      orientations(orientationCount, orientationData),
      morph(morphCount, morphData)
{
    weightCount = -1;
    cycle = owner;
}

PartCycle::~PartCycle()
{
    for (int i = 0; i < morph.count; i++) {
        WeightList *w = morph.Get(i);
        if (w != 0)
            delete w;
    }
}
