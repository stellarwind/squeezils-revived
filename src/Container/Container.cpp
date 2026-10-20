// Instantiations of the container templates, in the order the original has them in the binary (address order).
#define CONTAINER_DEFINITIONS
#include "Container.h"
#include "String/String.h"

template class Array<int>;
template class Array<String>;
