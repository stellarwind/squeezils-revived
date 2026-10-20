// Array<Vector3f>::Get (0x41c4b0), Array<_tag_PartOrientation>::Get (0x41d6e0): by-value return with an out-of-line dtor.
#define CONTAINER_DEFINITIONS
#include "Container/ContainerArray.h"
struct Vector3f { float x, y, z; Vector3f() {} Vector3f(const Vector3f& o) { x = o.x; y = o.y; z = o.z; } ~Vector3f(); Vector3f& operator=(const Vector3f& o) { x = o.x; y = o.y; z = o.z; return *this; } };
struct _tag_PartOrientation { Vector3f pos, dir; };
template class Array<Vector3f>;
template class Array<_tag_PartOrientation>;
