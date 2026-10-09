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

void SqueezilRoboSoul::ClearSeen()
{
    int *p = lastSeen;
    p[0] = -1;
    p[1] = -1;
    p[2] = -1;
}

void SqueezilRoboSoul::ResetSenses()
{
    for (int i = 0; i < 3; i++)
        records[i].Reset();
}

int SqueezilRoboSoul::IsNear(int kind)
{
    if (records[kind].id != -1) {
        float r = character->senseRadius[kind];
        return r * r > records[kind].distance;
    }
    return 0;
}
