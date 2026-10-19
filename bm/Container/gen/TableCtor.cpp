// Table<K,V,VArray,ID> constructor with int keys and a one-int-per-value array (SQZ.EXE 0x4650b0, 0x466590): members are built inline.
#define CONTAINER_DEFINITIONS
#include "Container/ContainerTable.h"
template class Table<int, int, PrimitiveArray<int, 1>, 21>;
