// BufferObjectTable<int, Character*/Creature*, BufferObjectArray<..>, ID> Create slots (SQZ.EXE 0x4653e0, 0x4668c0).
#define CONTAINER_DEFINITIONS
#include "Container2/PrimBuf.h"
#include "Container/ContainerTable.h"

#include "Container2/ObjectArrayViews.h"

template <class K, class V, class VArray, int ID>
BufferObject *BufferObjectTable<K, V, VArray, ID>::Create()
{
    return new BufferObjectTable<K, V, VArray, ID>;
}


template class BufferObjectTable<int, Character *, ObjArrayC, 5>;
template class BufferObjectTable<int, Creature *, ObjArrayD, 10>;
