// GoodieBody: the body of a goodie (a nut, a power-up lying in the world). Reconstructed from SQZ.EXE 0x45a850-0x45a9f0.
//
// It is a plain Body plus one scene frame: the goodie's prop (built from the template named in the level's prop tables) hangs on
// that frame, and every update puts the frame where the body's matrix says.
#pragma once
#include "Body.h"

struct IDirect3DRMFrame;

struct GoodieBody : Body {
    float collisionRadius;          // 0xb8 the simulation reads this on every actor, 0 meaning "does not collide" (the goodie never sets it)
    int   unused_bc;                // 0xbc
    IDirect3DRMFrame *instance;     // 0xc0 the goodie's frame in the scene, hung under the scene child

    // @0045a850 propName is the prop template to show (NULL: none; the frame is not made either)
    GoodieBody *Init(const float *matrix, const char *propName);
    void  Destruct();                       // @0045a940 takes the frame out of the scene
    GoodieBody *DestroyAndFree(int flags);  // @0045a920 slot 0 (bit 0 of flags: free the memory)
    void  UpdateFrame();                    // @0045a9d0 slot 2: the frame follows the matrix
    int   HasContact();                     // @0045a9f0 slot 8: whether the body stands on a surface face
};
