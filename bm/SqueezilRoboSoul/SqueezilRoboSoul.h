// SqueezilRoboSoul: a squeezil's robot soul. Only GetKind (0x4053d0) and Remember (0x404710) are ported.
#pragma once
#include "compat41.h"

struct AwarenessRecord {
    char pad14[0x14];
    int  stale;            // 0x14 1 after a reset
    void Reset();          // 0x4483c0: stale = 1
};

struct SqueezilRoboSoul {
    char pad8[0x8];
    int  finished;         // 0x08 (GrantMessageMap's field)
    char pad88[0x88 - 0xc];
    int  lastSeen[3];      // 0x88 the creature each kind's record held (-1: none)
    int  retarget;         // 0x94
    int  levelIndex;       // 0x98
    int  senseKind;        // 0x9c
    int  lastSenseKind;    // 0xa0
    AwarenessRecord records[3];   // 0xa4 the nearest player, robo, goodie

    int  GetKind();        // 0x4053d0 slot 8: 6 when finished, else 1
    void Remember();       // 0x404710: lastSenseKind = senseKind
    void ClearSeen();      // 0x404900: lastSeen[0..2] = -1
    void ResetSenses();    // 0x4046f0: records[i].Reset() for i in 0..2
};
