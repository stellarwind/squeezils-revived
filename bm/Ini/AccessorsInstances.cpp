// Instantiates Array<String>::At, which the INI classes call (kept apart from the main container instantiations; see Container/Accessors.h).
#define CONTAINER_DEFINITIONS
#include "Container/ContainerArray.h"
#include "String/String.h"
#include "Container/Accessors.h"

template String Array<String>::At(int);
