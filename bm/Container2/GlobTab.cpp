// Dynamic initializer of the global settings table at 0x4c7060 (SQZ.EXE 0x422620): the Table constructor stays out of line.
#include "String/String.h"
#include "Container/ContainerTable.h"

BufferObjectTable<String, int, PrimitiveArray<int, 1>, 21> g_settings;

