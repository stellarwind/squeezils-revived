// 0x45d430 / 0x45d5c0: register Character and Creature pointer-table prototypes.
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"

#include "Container2/ObjectArrayViews.h"
void RegisterBufferObject(BufferObject *prototype); // 0x448ce0

void RegisterBufferPrototype_45d430()
{
    RegisterBufferObject(new BufferObjectTable<int, Character *, ObjArrayC, 5>);
}
void RegisterBufferPrototype_45d5c0()
{
    RegisterBufferObject(new BufferObjectTable<int, Creature *, ObjArrayD, 10>);
}
