// AwarenessRecord::Reset (0x4483c0): mark the record stale.
#include "SqueezilRoboSoul/SqueezilRoboSoul.h"

void AwarenessRecord::Reset()
{
    stale = 1;
}
