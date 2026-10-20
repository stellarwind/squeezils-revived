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
