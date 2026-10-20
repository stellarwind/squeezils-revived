// 0x4106e0: destroy last frame's removed instances, then advance the removal queue.
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
struct FlushIds {
    virtual int Get(int index);
    virtual int Insert(const int &id, int index);
    virtual void Remove(int index);
    int count, capacity;
    int *data;
    void *bufferObject;
};
struct NavFlush {
    char pad[0x134];
    Array<FlushInstance *> instances;
    FlushIds removed, removedLater;
    void FlushRemoved();
};
void NavFlush::FlushRemoved()
{
    for (int k = removedLater.count - 1; k >= 0; --k) {
        int number = removedLater.Get(k);
        delete instances.Get(number);
        FlushInstance *none = 0;
        instances.Replace(number, none);
        removedLater.Remove(k);
    }
    for (int k = removed.count - 1; k >= 0; --k) {
        int number = removed.Get(k);
        removed.Remove(k);
        removedLater.Insert(number, -1);
    }
}
