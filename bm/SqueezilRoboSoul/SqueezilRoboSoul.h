// SqueezilRoboSoul: a squeezil's robot soul. Only GetKind (0x4053d0) and Remember (0x404710) are ported.
#pragma once
#include "compat41.h"

struct SqueezilRoboSoul {
    char pad8[0x8];
    int  finished;         // 0x08 (GrantMessageMap's field)
    char pad9c[0x9c - 0xc];
    int  senseKind;        // 0x9c
    int  lastSenseKind;    // 0xa0

    int  GetKind();        // 0x4053d0 slot 8: 6 when finished, else 1
    void Remember();       // 0x404710: lastSenseKind = senseKind
};
