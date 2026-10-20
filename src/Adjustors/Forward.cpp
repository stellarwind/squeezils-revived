// Secondary-interface entry points: recover the enclosing soul from this - 4.
#include "GoodieSoul/GoodieSoul.h"
struct RegionSoulEntry { void *ReturnNull(); }; // 0x438310
struct RegionSoulAdapter { void *ReturnNull(); }; // 0x4382c0
void *RegionSoulAdapter::ReturnNull()
{
    return ((RegionSoulEntry *)((char *)this - 4))->ReturnNull();
}
struct GoodieSoulAdapter { void *GetInterface(int which); }; // 0x45a7e0
void *GoodieSoulAdapter::GetInterface(int which)
{
    return ((GoodieSoul *)((char *)this - 4))->GetInterface(which);
}

// Scalar deleting destructors reached through the second base's vtable (the base at +4):
// the compiler's [thunk] entries, written out as adapters onto the primary scalar deleting destructor.
struct PlayerSoulDeleting { void *Delete(unsigned int flags); }; // 0x41ef60
struct PlayerSoulDeleteAdapter { void *Delete(unsigned int flags); }; // 0x41fae0
void *PlayerSoulDeleteAdapter::Delete(unsigned int flags)
{
    return ((PlayerSoulDeleting *)((char *)this - 4))->Delete(flags);
}
struct RegionSoulDeleting { void *Delete(unsigned int flags); }; // 0x4369c0
struct RegionSoulDeleteAdapter { void *Delete(unsigned int flags); }; // 0x4382b0
void *RegionSoulDeleteAdapter::Delete(unsigned int flags)
{
    return ((RegionSoulDeleting *)((char *)this - 4))->Delete(flags);
}
struct SqueezilSoulDeleting { void *Delete(unsigned int flags); }; // 0x44f570
struct SqueezilSoulDeleteAdapter { void *Delete(unsigned int flags); }; // 0x456d90
void *SqueezilSoulDeleteAdapter::Delete(unsigned int flags)
{
    return ((SqueezilSoulDeleting *)((char *)this - 4))->Delete(flags);
}
struct GoodieSoulDeleting { void *Delete(unsigned int flags); }; // 0x4596e0
struct GoodieSoulDeleteAdapter { void *Delete(unsigned int flags); }; // 0x45a7d0
void *GoodieSoulDeleteAdapter::Delete(unsigned int flags)
{
    return ((GoodieSoulDeleting *)((char *)this - 4))->Delete(flags);
}
