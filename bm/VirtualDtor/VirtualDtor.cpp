// Force emission of VirtualDtorObj's vtable (and its scalar deleting destructor).
#include "VirtualDtor/VirtualDtor.h"

VirtualDtorObj *VirtualDtorForceEmit()
{
    return new VirtualDtorObj;
}
