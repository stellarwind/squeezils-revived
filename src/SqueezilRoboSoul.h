// SqueezilRoboSoul: the soul of a squeezil that is not the player's (a robo squeezil, the level's computer-controlled creatures).
// Reconstructed from SQZ.EXE 0x4043a0-0x4053d0 (RTTI class SqueezilRoboSoul : SqueezilSoul, vtables 0x49d18c and 0x49d16c).
//
// It is a SqueezilSoul whose wishes come from a "behaviour" (an object of one of seven classes the original makes, not
// reconstructed yet: slot 1 of it is given the ground or air control record and answers with the state to go to) and whose
// moods come from a Character (the creature's template from the level file: SenseRadiusP / R / G and the names of the script
// functions the states run).
//
// How it decides. Every frame (UpdateRobo) it runs a little state machine (StepAI) that asks, in turn, for the squeezil's own
// state (did it leave or touch the ground; is it dying) and, for each of the three kinds of creature it can notice, for what to
// do about the nearest one of that kind. The kinds are the soul's slot 8 (0 a player, 1 another robo, 2 a goodie; 4 and 6 are
// creatures that do not count: dead or fixed): the creatures near enough are offered to it by the simulation (Sense) and it
// keeps the nearest of each kind (an AwarenessRecord). Each answer is a state number (5..0xc: noticed or lost the player / robo /
// goodie on the ground or in the air; 0xd..0x12: hit by one; 1 and 2: left and touched the ground; 3 and 4: dying), 0x13 meaning
// "nothing to do". The Character turns a state number into the name of a script function and the function is run; it decides
// the behaviour to use (SetBehaviour, slot 60 of the table).
//
// The InputEventTarget part does not exist (the object ends at 0xfc; its fields start at 0x80).
#pragma once
#include "SqueezilSoul.h"

struct Character;

// What the soul knows of the nearest creature of one kind (0x18 bytes, constructor @00404f10)
struct AwarenessRecord {
    int      id;            // 0x00 the creature (-1: none)
    Float3   position;      // 0x04 where it was
    float    distance;      // 0x10 how far it was (the simulation sorts by this)
    int      stale;         // 0x14 1 after a reset: the next offer is taken whatever its distance

    // @00448370 keeps the creature if it is nearer than the one held (or the one held is stale)
    void Offer(const AwarenessRecord *other);
    void Reset() { stale = 1; }                     // @004483c0
};
static_assert(sizeof(AwarenessRecord) == 0x18, "AwarenessRecord layout");

enum RoboKind { ROBO_PLAYER = 0, ROBO_ROBO = 1, ROBO_GOODIE = 2 };
const int kNoActivity = 0x13;       // "no state to go to"
const int kNoSense = 0x3e7;         // "no creature noticed"

struct SqueezilRoboSoul : SqueezilSoul {
    int              step;              // 0x80 which question StepAI asks next (0: the soul's own state, 1-3: the kinds)
    int              wasOnGround;       // 0x84 the body's on-ground flag as of the last PickState (1 at the start)
    int              lastSeen[3];       // 0x88 the creature each kind's record held when it was last looked at (-1: none)
    int              retarget;          // 0x94 cleared every frame; nothing here sets it
    char             unknown98[4];      // 0x98
    int              senseKind;         // 0x9c the kind (0-2) just noticed or hit by, kNoSense for none
    int              lastSenseKind;     // 0xa0 the value of senseKind when a state was last taken (Remember)
    AwarenessRecord  records[3];        // 0xa4 the nearest player, robo, goodie
    void            *behaviour;         // 0xec the current behaviour object (NULL: none)
    int              mode;              // 0xf0 which behaviour class it is (0-6; 7 at the start: none)
    Character       *character;         // 0xf4
    const char      *pendingFunction;   // 0xf8 the script function the Character chose for the state taken, until it is run
    // total 0xfc

    SqueezilRoboSoul *Init(SqueezilBody *body, Possessions *possessions, void *place, Character *character);   // @004043a0 ret 0x10
    void  Destruct();                                       // @00404510
    SqueezilRoboSoul *DestroyAndFree(int flags);            // @004044a0 slot 0
    void  UpdateRobo();                                     // @00404df0 slot 5
    void  Sense(const AwarenessRecord *seen);               // @00404720 slot 6 ret 4: a creature is near; kept if it is the nearest of its kind
    void  ResetSenses();                                    // @004046f0 slot 7: the three records go stale
    int   GetKind() const;                                  // @004053d0 slot 8: 1 while it plays, 6 when it is finished
    void  Hit(int knock, int sprayNuts, int byId);          // @00404630 slot 9
    void  ReadGroundInput(GroundControl *c, int *next);     // @004045b0 slot 39
    void  ReadAirInput(AirControl *c, int *next);           // @004045f0 slot 40
    void  SetBehaviour(int mode, const float *args);        // @00404b10 slot 60 ret 8

    void  Remember() { lastSenseKind = senseKind; }         // @00404710
    int   IsNear(int kind);                                 // @00404770 ret 4: the kind's record is within the Character's sense radius
    int   TargetGone(int kind);                             // @004047c0 ret 4: the record's creature is gone or finished
    int   PickState();                                      // @00404840 the soul's own state: dying, left or touched the ground
    void  ClearSeen();                                      // @00404900
    int   SenseState(int kind);                             // @00404920 ret 4: what to do about the kind's nearest creature
    int   StepAI();                                         // @00404a80 asks the questions in turn until one gives a state
    // Not here: GrantMessageMap slots @00404f80 (a thunk to the destructor), @00404f90 (a thunk to GetInterface), @00404fa0 and
    // @00404fb0 (return 0); slots 17 @00404fc0 and 10 @00404fd0 are the base class's.
};
static_assert(sizeof(SqueezilRoboSoul) == 0xfc, "SqueezilRoboSoul layout");
