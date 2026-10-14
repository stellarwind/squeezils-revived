// MorphCycle dtor (0x41b940): when not sharing data, free the part tracks, the parts table, and the sample times.
#include "Container/ContainerTable.h"
#include "PartCycle/PartCycle.h"
#include "String/String.h"
#include "MorphCycle/MorphCycle.h"

typedef Table<String, PartCycle *, Array<PartCycle *>, 0> PartCycleTable;

MorphCycle::~MorphCycle()
{
    if (sharesData == 0) {
        int offset = 0;
        for (int i = 0; ; offset += 4, i++) {
            if (i >= ((PartCycleTable *)parts)->keys.count) break;
            PartCycle *pc = *(PartCycle **)((char *)((PartCycleTable *)parts)->values.data + offset);
            if (pc != 0) delete pc;
        }
        delete (PartCycleTable *)parts;
        delete (PrimitiveArray<float, 2> *)times;
    }
}


