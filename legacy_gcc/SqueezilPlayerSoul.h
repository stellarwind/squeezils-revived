// SqueezilPlayerSoul: the soul of the player's squeezil while it is in a place (PlaceSimulation.h). Reconstructed from SQZ.EXE
// 0x41ee30-0x41fb80 (RTTI class SqueezilPlayerSoul, vtables 0x49d91c, 0x49d8fc and 0x49d8d4).
//
// It is a SqueezilSoul whose wishes come from the keyboard: every frame the base class asks it for the ground or the air input
// (slots 39 and 40) and it reads the application's axes (forward, turn), the one-press "taps" and the held buttons, and turns them
// into the control record and the state the player wishes to go to. It also
//   - puts the squeezil to sleep when it has stood still for PlayerSleepTime seconds ("sleepEffect" plays), and makes the landing
//     sound ("landEffect") when the body touches the ground,
//   - counts the food nuts it carries against FoodNutsToFinish and runs the script function EnoughNutsFunction once there are
//     enough (PostUpdate),
//   - finishes the level (Finish): the nuts are banked, the place is marked completed in the inventory and the game goes back to
//     the region the player came from; after death the same call starts the place over,
//   - takes the keys: 0x16 leave (after the death is over), 0x1e back to the level's start, 0x1f freeze or unfreeze.
//
// The object is a SqueezilSoul with the input event target (vtable 0x49d8d4) at +0x80. InputEventTarget slots (`this` there is the
// soul + 0x80): 0 nothing, 1 OnEvent(what, text), 2 to 4 nothing, 5 the jump button went down, 6 it was let go (strength), 7 the
// spit button went down, 8 it was let go (strength).
#pragma once
#include "SqueezilSoul.h"

struct SqueezilPlayerSoul : SqueezilSoul {
    void           **inputVtbl;         // 0x80 @0049d8d4 the InputEventTarget part
    int              attackHeld;        // 0x84 the attack / dive button, as the last ground frame found it; in the air it is what tells a
                                        //      press made in the air (dive) from one held since the ground
    int              unknown88;         // 0x88 -1, not used here
    char             unknown8c[4];      // 0x8c
    int              unknown90;         // 0x90 0, not used here
    char             unknown94[4];      // 0x94
    int              enoughNuts;        // 0x98 EnoughNutsFunction has run
    float            sleepTimer;        // 0x9c seconds the squeezil has stood still
    int              wasOnGround;       // 0xa0 the body's on-ground flag as of the last frame (1 at the start)
    // total 0xa4

    SqueezilPlayerSoul *Init(SqueezilBody *body, Possessions *possessions, void *place);   // @0041ee30 ret 0xc
    void  Destruct();                                       // @0041ef80
    SqueezilPlayerSoul *DestroyAndFree(int flags);          // @0041ef60 slot 0
    void  UpdatePlayer();                                   // @0041f5a0 slot 5 (named UpdatePlayer: SqueezilSoul::Update is another function)
    void  Hit(int knock, int sprayNuts, int unused);        // @0041fab0 slot 9: the looping sound stops, then the base class's
    void  ReadGroundInput(GroundControl *c, int *next);     // @0041f1c0 slot 39
    void  ReadAirInput(AirControl *c, int *next);           // @0041f3e0 slot 40
    int   PostUpdate();                                     // @0041f750 once a frame after the souls: the nuts are enough (1) or not (0)
    void  Finish();                                         // @0041f7f0 leave the finished level, or start it over when the player is dead

    // InputEventTarget part (the hooks give the soul: they step back from soul + 0x80)
    void  OnEvent(int what, const char *text);              // @0041eff0 slot 1, ret 8
    void  JumpPressed();                                    // @0041f070 slot 5
    void  JumpReleased(float strength);                     // @0041f160 slot 6, ret 4
    void  SpitPressed();                                    // @0041f180 slot 7
    void  SpitReleased(float strength);                     // @0041f1a0 slot 8, ret 4
    // Not here (one to three instructions long): slot 6 @0041fb50, slot 7 @0041fb60, slot 14 @0041f7e0 do nothing; slot 8 @0041fb70
    // returns 6 once the soul is finished and 0 before; GrantMessageMap slot 2 @0041fb00 returns 1; the InputEventTarget slots 0, 2, 3
    // and 4 @0041fb10-0041fb40 do nothing.
};
static_assert(sizeof(SqueezilPlayerSoul) == 0xa4, "SqueezilPlayerSoul layout");
