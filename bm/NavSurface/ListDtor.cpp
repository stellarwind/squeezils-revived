// 0x40d640: destroy loaded surfaces and instances, then release the list members.
#include "Container/ContainerArray.h"
struct FlushMatrix { float m[16]; ~FlushMatrix(); }; // 0x402310
struct FlushVector { float x, y, z; ~FlushVector(); }; // 0x401660
struct FlushProperties { char data[0x30]; ~FlushProperties(); }; // 0x40c3d0
struct FlushFlags { char data[0x14]; ~FlushFlags(); }; // 0x40c420
struct FlushInstance {
    void *surface;
    FlushMatrix matrix;
    int identity;
    FlushMatrix previous;
    int previousIdentity, moveDirty, changed;
    void *prop;
    FlushProperties properties;
    FlushFlags flags;
    FlushVector position, previousPosition;
};
#include "Container/ContainerSet.h"
#include "Container/ContainerTable.h"
#include "String/String.h"
struct DestroyLoadedSurface { ~DestroyLoadedSurface(); }; // 0x40bee0
struct DestroySurfaceType;
typedef Set<String, 11> DestroyNameSet;
typedef Set<int, 10> DestroyIdSet;
typedef Table<String, DestroySurfaceType *, Array<DestroySurfaceType *>, 0> DestroyTypeTable;
struct NavSurfaceDestroy {
    char path[0x100];
    DestroyNameSet knownNames;
    Array<DestroyNameSet *> fileNames;
    Array<DestroyLoadedSurface *> loaded;
    Array<FlushInstance *> instances;
    DestroyIdSet removed, removedLater;
    DestroyTypeTable types;
    ~NavSurfaceDestroy();
};
NavSurfaceDestroy::~NavSurfaceDestroy()
{
    for (int i = loaded.count - 1; i >= 0; --i) {
        delete loaded.Get(i);
        loaded.Remove(i);
    }
    for (int i = instances.count - 1; i >= 0; --i) {
        if (instances.Get(i))
            delete instances.Get(i);
        instances.Remove(i);
    }
}
