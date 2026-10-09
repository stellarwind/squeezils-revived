// SqueezilBody::IsOnSurface (0x459000): the same as slot 15 (IsOnGround), a virtual tail-call.
#include "SqueezilBodyVtbl/SqueezilBodyVtbl.h"

int SqueezilBody::IsOnSurface()
{
    return IsOnGround();
}
