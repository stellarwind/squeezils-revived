// PropList: see PropList.h. Reconstructed from SQZ.EXE 0x42d010-0x42d2b0.
#include "PropList.h"
#include "Prop.h"
#include "PropLoader.h"
#include "Animation.h"

#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"

static Array<Prop *> *const kUpdating = (Array<Prop *> *)0x4c71f8;    // the props called every frame
static Array<int>    *const kRemoved  = (Array<int> *)0x4c72a8;       // places on that list queued for removal
static Array<Prop *> *const kDestroy  = (Array<Prop *> *)0x4c72e8;    // props to delete when their place is cleared away

static PropTable *const kSecondProps = (PropTable *)0x4c7230;
static PathTable *const kSecondPaths = (PathTable *)0x4c7258;
static PropTable *const kThirdProps  = (PropTable *)0x4c72c0;
static PathTable *const kThirdPaths  = (PathTable *)0x4c71d0;

// @0042d180
void AddUpdatingProp(Prop *prop)
{
    kUpdating->Insert(prop);
}

// @0042d1a0 (the whole list is searched: every place the prop is on is queued)
void RemoveUpdatingProp(Prop *prop)
{
    for (int i = kUpdating->count - 1; i >= 0; i--) {
        if (kUpdating->At(i) == prop)
            kRemoved->Insert(i);
    }
}

// @0042d1f0
void DestroyPropLater(Prop *prop)
{
    kDestroy->Insert(prop);
}

// @0042d210
void UpdateProps(float seconds)
{
    FlushRemovedProps();
    for (int i = 0; i < kUpdating->count; i++)
        CallVirtual<void>(kUpdating->At(i), 7, seconds);
    FlushRemovedProps();
}

// @0042d250
void UpdateLevels()
{
    FlushRemovedProps();
    for (int i = 0; i < kUpdating->count; i++) {
        int queued = 0;
        while (queued < kRemoved->count && kRemoved->At(queued) != i)
            queued++;
        if (queued == kRemoved->count)
            CallVirtual<void>(kUpdating->At(i), 8);
    }
    FlushRemovedProps();
}

// @0042d2b0 Queued places are taken from the end of the queue; a prop on the destroy list is deleted on the way.
void FlushRemovedProps()
{
    for (int queued = kRemoved->count - 1; queued >= 0; queued--) {
        int place = kRemoved->At(queued);
        for (int j = kDestroy->count - 1; j >= 0; j--) {
            Prop *prop = kUpdating->At(place);
            if (kDestroy->At(j) != prop)
                continue;
            if (prop)
                prop->VDestroy(1);
            kDestroy->RemoveAt(j);
        }
        kUpdating->RemoveAt(place);
        kRemoved->RemoveAt(queued);
    }
}

// Deletes every prop template in a table and empties the table (from the end), as ClearPropTables does for the first pair
static void EmptyPropTable(PropTable *table)
{
    int count = table->keys.count;
    for (int i = 0; i < count; i++) {
        Prop *prop = table->values.At(i);
        if (prop)
            prop->VDestroy(1);
    }
    for (int i = 0; i < count; i++)
        table->RemoveAt(count - i - 1);
}

static void EmptyPathTable(PathTable *table)
{
    int count = table->keys.count;
    for (int i = 0; i < count; i++) {
        MorphCycle *path = table->values.At(i);
        if (path)
            delete path;
    }
    for (int i = 0; i < count; i++)
        table->RemoveAt(count - i - 1);
}

// @0042d010 (the original also made a String of every key on the way: no effect)
void ClearWorldObjects()
{
    EmptyPropTable(kThirdProps);
    EmptyPathTable(kThirdPaths);
    EmptyPropTable(kSecondProps);
    EmptyPathTable(kSecondPaths);
    FlushRemovedProps();
    for (int i = kDestroy->count - 1; i >= 0; i--) {
        Prop *prop = kDestroy->At(i);
        if (prop)
            prop->VDestroy(1);
        kDestroy->RemoveAt(i);
    }
    for (int i = kUpdating->count - 1; i >= 0; i--)
        kUpdating->RemoveAt(i);
}
