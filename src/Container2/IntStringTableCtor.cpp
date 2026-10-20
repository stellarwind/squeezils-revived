// 0x4635e0: integer-keyed string table constructor.
#include "String/String.h"
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"

template class Table<int, String, PrimitiveArray<String, 3>, 24>;
