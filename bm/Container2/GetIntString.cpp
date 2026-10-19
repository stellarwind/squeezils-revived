// SQZ.EXE 0x4638b0: Table<int, String, PrimitiveArray<String,3>,24>::Get.
// Keep At out of line (0x428af0); key lookup dispatches through Set::IndexOf.
#include "String/String.h"
#define CONTAINER_DEFINITIONS
#include "Container/ContainerTable.h"

template String Table<int, String, PrimitiveArray<String, 3>, 24>::Get(const int &key);
