// 0x45e9b0: Vector3f array growth, using the shared Array implementation.
#include "Vector3f/Vector3f.h"
#define CONTAINER_DEFINITIONS
#include "Container/ContainerArray.h"

template class Array<Vector3f>;
