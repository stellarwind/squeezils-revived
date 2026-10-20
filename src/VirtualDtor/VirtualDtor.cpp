// Force emission of VirtualDtorObj's vtable (and its scalar deleting destructor).
#include "VirtualDtor/VirtualDtor.h"

VirtualDtorObj *VirtualDtorForceEmit()
{
    return new VirtualDtorObj;
}

void FlatDtor::Dtor()
{
    vtbl = (void **)0x49d644;   // the vtable address is masked to X by flagsearch
}
