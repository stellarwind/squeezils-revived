// Keylist accessor leaves (0x43d620 / 0x43d610 / 0x43d600).
#include "Keylist/Keylist.h"

void *Keylist::Slot5(int i)
{
    return field_4 + i * 20;
}

void *Keylist::Slot6(int)
{
    return field_4;
}

void *Keylist::Slot7(int)
{
    return field_4;
}
