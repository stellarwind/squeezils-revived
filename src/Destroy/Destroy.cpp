// DestroyObj::DestroyAndFree — the shared slot-0 pattern.
#include "Destroy/Destroy.h"

DestroyObj *DestroyObj::DestroyAndFree(int flags)
{
    Destruct();
    if (flags & 1)
        operator delete(this);
    return this;
}
