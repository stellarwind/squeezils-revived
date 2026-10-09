// SqueezilRoboSoul GetKind (0x4053d0) and Remember (0x404710).
#include "SqueezilRoboSoul/SqueezilRoboSoul.h"

int SqueezilRoboSoul::GetKind()
{
    return finished ? 6 : 1;
}

void SqueezilRoboSoul::Remember()
{
    lastSenseKind = senseKind;
}
