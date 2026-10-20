// RemoveUpdatingProp, SQZ.EXE 0x42d1a0.
#include "PropList/UpdatingProps.h"

void RemoveUpdatingProp(Prop *prop)
{
    for (int index = gUpdatingProps.count - 1; index >= 0; --index) {
        if (gUpdatingProps.data[index] == prop)
            gRemovedPropIndices.Insert(index, -1);
    }
}
