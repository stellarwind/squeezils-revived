// CompassRuler/Panel leaf wrappers (SQZ.EXE): the UpdateSprites thunk (0x4153e0) and Panel::UpdateRegionSprites
// (0x42e2a0, a tail-call into the compass embedded at Panel + 0x480).
#pragma once
#include "compat41.h"

struct CompassRuler {
    void UpdateSpritesReal();   // 0x414af0 (the real work; out-of-line, called)
    int  UpdateSprites();       // 0x4153e0 (thunk: call real, return 1)
};

struct Panel {
    char pad[0x480];
    CompassRuler compass;       // 0x480 (embedded)
    int UpdateRegionSprites();  // 0x42e2a0 (tail-call)
};
