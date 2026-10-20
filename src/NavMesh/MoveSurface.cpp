// SQZ.EXE 0x40c470: retain the old placement, install the new one and update its origin.
#include <string.h>
#include "NavMesh/SurfaceTransform.h"

void InstancedSurface::Move(const Matrix16 &next)
{
    previous = matrix;
    previousIdentity = identity;
    previousPosition = position;
    moveDirty = 1;
    matrix = next;
    Matrix16 identityMatrix;
    identity = memcmp(matrix.v, identityMatrix.v, 0x40) == 0;
    Vector3f origin(0.0f, 0.0f, 0.0f);
    position = ToWorld(origin);
}
