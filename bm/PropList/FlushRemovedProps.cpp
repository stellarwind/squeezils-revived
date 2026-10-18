// FlushRemovedProps, SQZ.EXE 0x42d2b0.
#include "PropList/UpdatingProps.h"

struct DestroyableProp {
    virtual void Destroy(int flags); // slot 0, scalar deleting destructor
};
extern UpdatingPropArray gDestroyProps; // 0x4c72e8

void FlushRemovedProps()
{
    for (int queued = gRemovedPropIndices.count - 1; queued >= 0; --queued) {
        int index = gRemovedPropIndices.data[queued];
        int destroyed = gDestroyProps.count;
        for (;;) {
            --destroyed;
            if (destroyed < 0)
                break;
            Prop *prop = gUpdatingProps.data[index];
            if (gDestroyProps.data[destroyed] != prop)
                continue;
            if (prop)
                ((DestroyableProp *)prop)->Destroy(1);
            gDestroyProps.Remove(destroyed);
        }
        gUpdatingProps.Remove(index);
        gRemovedPropIndices.Remove(queued);
    }
}
