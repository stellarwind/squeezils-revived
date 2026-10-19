// 0x464220: float-keyed string table constructor.
#include "String/String.h"
#define CONTAINER_DEFINITIONS
#define BUFFEROBJECT_SIGNATURES
#define SET_CREATE
#include "Container/ContainerTable.h"

template class Table<float, String, PrimitiveArray<String, 3>, 25>;
