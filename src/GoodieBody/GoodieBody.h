// GoodieBody: a plain Body plus one scene frame. Only HasContact (0x45a9f0) is ported. Layout from Repo/legacy_gcc/GoodieBody.h.
#pragma once
#include "Body/Body.h"

struct GoodieBody : Body {
    float collisionRadius;    // 0xb8
    int   unused_bc;          // 0xbc
    void *instance;           // 0xc0

    int HasContact();         // 0x45a9f0 slot 8
};
