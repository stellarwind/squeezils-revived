// PointNearSegment (SQZ.EXE 0x413100): is the point within sqrt(maxDistSq) of the line origin->end, measured over the first unit of length.
#include "Vector3f/Vector3f.h"

int __stdcall PointNearSegment(const Vector3f *point, const Vector3f *origin, const Vector3f *end, float maxDistSq)
{
    Vector3f direction = *end - *origin;
    direction.Normalize();
    float negativeDot = -(point->x * direction.x + point->y * direction.y + point->z * direction.z);
    float along = -(negativeDot + (origin->z * direction.z + origin->y * direction.y + origin->x * direction.x)) / (direction.y * direction.y + direction.z * direction.z + direction.x * direction.x);
    if (along < 0.0f || along > 1.0f)
        return 0;
    Vector3f nearest = *origin + direction * along;
    return (*point - nearest).LengthSq() < maxDistSq;
}
