// InstancedSurface::GetProperties (0x40d520): the property table at +0x98.
#include "NavSurface/NavSurfaceList.h"

void *InstancedSurface::GetProperties()
{
    return (char *)this + 0x98;
}
