// Array<String> (SQZ.EXE 0x45e0e0-0x45e490, Get at 0x40ec10), one instantiation in its own translation unit.
#define CONTAINER_DEFINITIONS
#define ARRAY_GET_VIA_LOCAL
#include "Container/ContainerArray.h"
#include "String/String.h"

template class Array<String>;
