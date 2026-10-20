// SQZ.EXE 0x45c2e0 / 0x45c3c0: register empty integer and float array prototypes.
#define CONTAINER_DEFINITIONS
#include "Container/ContainerSet.h"

void RegisterBufferObject(BufferObject *prototype);      // 0x448ce0

void RegisterBufferPrototype_45c2e0()
{
    RegisterBufferObject(new PrimitiveArray<int, 1>);
}

void RegisterBufferPrototype_45c3c0()
{
    RegisterBufferObject(new PrimitiveArray<float, 2>);
}
