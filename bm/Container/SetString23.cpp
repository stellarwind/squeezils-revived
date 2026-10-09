// Set<String,23> (SQZ.EXE 0x462380 Insert, 0x4626f0 Contains, 0x462710 IndexOf, 0x4627c0 RemoveKey, 0x4627e0 Replace).
#define CONTAINER_DEFINITIONS
#define SET_INSERT_EXPERIMENT
#include "Container/ContainerSet.h"
#include "String/String.h"

template class Set<String, 23>;
