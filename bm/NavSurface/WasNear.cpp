// SQZ.EXE 0x40c7b0: test a point against the position before the last Move.
#include "compat41.h"

struct PreviousNearVector {
    float x, y, z;
    ~PreviousNearVector();                              // 0x401660
    PreviousNearVector operator-(const PreviousNearVector &point); // 0x401ae0
};
struct PreviousNearData {
    void *mesh;
    float sizeSquared;
};
struct PreviousNearSurface {
    char pad00[0xe8];
    PreviousNearVector previousPosition;
    PreviousNearData *GetSurface();                     // 0x40c6b0
    int WasNear(const PreviousNearVector &point);
};

int PreviousNearSurface::WasNear(const PreviousNearVector &point)
{
    PreviousNearVector delta = previousPosition - point;
    float squared = delta.x * delta.x + delta.z * delta.z + delta.y * delta.y;
    if (!(GetSurface()->sizeSquared < squared))
        return 1;
    return 0;
}
