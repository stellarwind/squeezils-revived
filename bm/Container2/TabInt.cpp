// BufferObjectTable<K, V, VArray, ID> Create slots (SQZ.EXE 0x460540 and siblings).
#include "String/String.h"
#define PB_STRING
#include "Container2/PrimBuf.h"
#include "Container/ContainerTable.h"

template <class K, class V, class VArray, int ID>
BufferObject *BufferObjectTable<K, V, VArray, ID>::Create()
{
    return new BufferObjectTable<K, V, VArray, ID>;
}

template class BufferObjectTable<int, int, PrimitiveArray<int, 1>, 20>;
template class BufferObjectTable<String, int, PrimitiveArray<int, 1>, 21>;
template class BufferObjectTable<String, float, PrimitiveArray<float, 2>, 22>;
template class BufferObjectTable<int, String, PrimitiveArray<String, 3>, 24>;
template class BufferObjectTable<float, String, PrimitiveArray<String, 3>, 25>;
template class BufferObjectTable<String, String, PrimitiveArray<String, 3>, 23>;
