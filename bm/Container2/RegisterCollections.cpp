// Startup registrations for string/vector arrays, string sets and primitive tables.
#include "String/String.h"
#include "Vector3f/Vector3f.h"
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"

void RegisterBufferObject(BufferObject *prototype); // 0x448ce0

void RegisterBufferPrototype_45c4a0()
{
    RegisterBufferObject(new PrimitiveArray<String, 3>);
}
void RegisterBufferPrototype_45c640()
{
    RegisterBufferObject(new PrimitiveArray<Vector3f, 4>);
}
void RegisterBufferPrototype_45c850()
{
    RegisterBufferObject(new Set<String, 11>);
}
void RegisterBufferPrototype_45c980()
{
    RegisterBufferObject(new BufferObjectTable<int, int, PrimitiveArray<int, 1>, 20>);
}
void RegisterBufferPrototype_45cb00()
{
    RegisterBufferObject(new BufferObjectTable<String, int, PrimitiveArray<int, 1>, 21>);
}
void RegisterBufferPrototype_45ccd0()
{
    RegisterBufferObject(new BufferObjectTable<String, float, PrimitiveArray<float, 2>, 22>);
}
void RegisterBufferPrototype_45cea0()
{
    RegisterBufferObject(new BufferObjectTable<String, String, PrimitiveArray<String, 3>, 23>);
}
void RegisterBufferPrototype_45d0b0()
{
    RegisterBufferObject(new BufferObjectTable<int, String, PrimitiveArray<String, 3>, 24>);
}
void RegisterBufferPrototype_45d270()
{
    RegisterBufferObject(new BufferObjectTable<float, String, PrimitiveArray<String, 3>, 25>);
}
