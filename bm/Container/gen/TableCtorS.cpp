// Table<K,V,VArray,ID> constructor with String keys and an Array<int> (or Array<float>) of values (SQZ.EXE 0x4655a0, 0x468210, 0x469df0, 0x46a8b0, 0x46acf0, 0x46b2e0, 0x46b930, 0x46cff0): members are built inline.
#define CONTAINER_DEFINITIONS
#include "Container/ContainerTable.h"
#include "String/String.h"
template class Table<String, int, Array<int>, 21>;
