// GoodieSoul: the soul of a goodie (a nut or power-up) lying in, or flying through, a place. Reconstructed from SQZ.EXE
// 0x459590-0x45a840 (RTTI class GoodieSoul : PhysicalSoul, vtables 0x49ff44 and 0x49ff24). Its body is a GoodieBody (GoodieBody.h).
//
// What it does. A goodie sprayed from a squeezil (Spray) or thrown by one (Launch) flies on a ballistic arc: each frame (Update) the
// displacement is direction * speed * dt plus the gravity term, a flight test against the place's surfaces (FlightHitsSurface)
// decides whether it hits one, and a hit bounces it (the direction is reflected in the face's normal, the speed damped) until
// BounceCount bounces are used up, when it lands for good on the face it hit. A goodie standing still has no direction: Update then
// does nothing. A goodie that falls below GoodieRebudPlane, or whose surface "rebuds goodies" (the delay runs out), is removed and
// the place's level file is told, so that it may put the goodie back.
//
// When a creature with a GrantMessageMap interface (a squeezil) touches it (Touch), the goodie is given to it (Grant: the goodie's
// name and its own creature id) and removed; a goodie that was thrown by the same creature is not (it bounces off instead).
//
// The soul is not a SqueezilSoul: PhysicalSoul's two vtable words, then the fields below (0x34 bytes in all). The tunables come from
// the [GoodiePhysics] section of the ini file (read by the first goodie made).
#pragma once
#include "GameString.h"
#include "Vector3f.h"
#include "Float3.h"
#include "SqueezilSoul.h"

struct GoodieBody;

// The settings (a static object at 0x4c7bc0; the global at 0x4c7bb4 points to it)
struct GoodieSettings {
    PhysicalSettings physical;      // 0x00 a copy of PhysicalSoul's
    float gravityFactor;            // 0x24 GoodieGravityFactor (1): how much of the gravity pulls a goodie
    int   bounceCount;              // 0x28 BounceCount (3): bounces before it lands for good
    float bounceDamping;            // 0x2c BounceDamping (0.5): what is left of the speed after a bounce
    float collisionSpeedAmp;        // 0x30 CollisionSpeedAmp (1): the body's +0xbc (the collision code reads it)
    float sprayAngle;               // 0x34 AngleOfSpray (45): degrees up from level
    float spraySpeed;               // 0x38 InitialSpraySpeed (5)
    float sprayOffset;              // 0x3c InitialSprayPosOffset (1): how far ahead of the squeezil a sprayed goodie starts
};
static_assert(sizeof(GoodieSettings) == 0x40, "GoodieSettings layout");

struct Matrix16;
struct Possessions;
struct PlaceSimulation;

struct GoodieSoul {
    void       **vtbl;          // 0x00 @0049ff44
    void       **grantVtbl;     // 0x04 @0049ff24 the GrantMessageMap part
    GoodieBody  *body;          // 0x08
    String       name;          // 0x0c the goodie's name: what an inventory keeps it under
    int          kind;          // 0x10 from the level file's goodie record; 0 for a goodie that can be knocked about and picked up from the
                                //      air, otherwise one that does not (its speed is held at 0 and Hit does nothing to it)
    int          thrown;        // 0x14 set by Launch, cleared by a landing, a Hit or a touch that did not give it away: it counts as
                                //      attacking (a spat nut) and is not given to a creature while it is set
    int          bounces;       // 0x18 bounces so far
    float        flightTime;    // 0x1c seconds of the arc so far (0 after a bounce)
    Vector3f     direction;     // 0x20 the direction of flight; zero when it lies still
    float        rebudTimer;    // 0x2c seconds since it landed on a rebud face; -1 when it is not counting
    float        rebudDelay;    // 0x30 how long it may stay there (RebudGoodies of the face's type)
    // total 0x34

    GoodieSoul *Init(GoodieBody *body, const char *name, int kind);   // @00459590 ret 0xc
    GoodieSoul *DestroyAndFree(int flags);                  // @004596e0 slot 0, ret 4
    void *GetInterface(int which);                          // @00459960 slot 1: the GrantMessageMap part for 1
    int   IsLoose() const;                                  // @00459980 slot 3: kind is 0
    void  Touch(int otherId);                               // @004599b0 slot 4: another creature ran into the goodie
    void  UpdateGoodie();                                   // @00459f80 slot 5
    void  Hit(int knock, int unused, int byId);             // @00459a80 slot 9: ret 0xc; a knocked goodie flies off the way the hitter was going
    int   GetKind() const;                                  // @00459bf0 slot 8: 2 for a loose goodie, 4 otherwise
    GoodieSettings *GetSettings();                          // @0045a840 slot 10
    int   IsAttacking() const;                              // @0045a830 slot 11: thrown
    void  Spray(Matrix16 from);                             // @00459c00 ret 0x40: fly off the squeezil's matrix on the next of 16 sprayed courses
    void  Launch(Matrix16 from, Float3 direction, float speed);   // @00459ed0 ret 0x50: thrown from there (a spat nut)

    // GrantMessageMap part (`this` is the soul + 4 there, the hooks step back)
    int   GrantSlot3(int arg) const;                        // @00459990 slot 3: the goodie is free to be given away
    int   GrantTo(int otherId);                             // @00459b80 slot 5: give the goodie to that creature; 1 when it took it
    // slots 6 @0045a800 and 7 @0045a810 (the main table) do nothing; the grant table's slots 2 @0045a7f0 and 4 @0045a820 return 0
};
static_assert(sizeof(GoodieSoul) == 0x34, "GoodieSoul layout");

// @004597b0 Reads [GoodiePhysics] into the settings (the constructor calls it for the first goodie made)
void ReadGoodieSettings();
