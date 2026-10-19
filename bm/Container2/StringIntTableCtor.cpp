// 0x460f50: string-keyed integer table constructor.
#include "String/String.h"
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"

template class Table<String, int, PrimitiveArray<int, 1>, 21>;
