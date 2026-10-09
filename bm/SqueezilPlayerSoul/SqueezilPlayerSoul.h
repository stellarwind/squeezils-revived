// SqueezilPlayerSoul: the player's soul. Only GetKind (0x41fb70, returns 6 once finished, else 0) is ported.
#pragma once
#include "compat41.h"

struct SqueezilPlayerSoul {
    char pad8[0x8];
    int  finished;   // 0x08 (GrantMessageMap's field)

    int GetKind();   // 0x41fb70 slot 8
};
