// Array<int> constructors not covered elsewhere: (n, src) at 0x45dcd0 and the copy constructor at 0x45dc70.
#define ARRAY_COPY_CTOR
#define CONTAINER_DEFINITIONS
#include "Container/ContainerArray.h"

template class Array<int>;
