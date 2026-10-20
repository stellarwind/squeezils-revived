// 0x45c760: register the integer-ID set prototype.
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerSet.h"

void RegisterBufferObject(BufferObject *prototype); // 0x448ce0

void RegisterBufferPrototype_45c760()
{
    RegisterBufferObject(new Set<int, 10>);
}
