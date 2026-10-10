// Set<T,ID> complete-object-destructor (2 vtable stores + base dtor), shared by every Set instantiation.
#define CONTAINER_DEFINITIONS
#define SET_DTOR
#include "Container/ContainerSet.h"

template class Set<int, 10>;
