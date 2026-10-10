// Table<K,V,VArray,ID> complete-object-destructor (vtable + values@+0x18 + keys@+4), shared by every instantiation.
#define CONTAINER_DEFINITIONS
#include "Container/ContainerTable.h"
#include "String/String.h"

template class Table<String, int, PrimitiveArray<int, 1>, 21>;
