// SQZ.EXE 0x40c700: is a world-space point within the surface's cached radius?
#include "compat41.h"

struct NearVector {
    float x, y, z;
    NearVector() {}
    ~NearVector();                          // 0x401660
    NearVector operator-(const NearVector &other); // 0x401ae0
};
struct NearSurfaceData {
    void *mesh;
    float sizeSquared;                      // +0x4
};
struct NearSurfaceInstance {
    char pad00[0xdc];
    NearVector position;
    NearSurfaceData *GetSurface();          // 0x40c6b0
    int IsNear(const NearVector &point);
};

// Keep the difference vector and its squared length in one contiguous local record.
struct NearDistanceWork {
    NearVector delta;
    float squared;
    NearDistanceWork(NearVector &position, const NearVector &point)
        : delta(position - point) {}
};

int NearSurfaceInstance::IsNear(const NearVector &point)
{
    NearDistanceWork work(position, point);
    work.squared = work.delta.x * work.delta.x + work.delta.y * work.delta.y + work.delta.z * work.delta.z;
    if (!(GetSurface()->sizeSquared < work.squared))
        return 1;
    return 0;
}
