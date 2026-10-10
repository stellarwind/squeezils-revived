// CreatureTemplate: what the level file says about one kind of robo creature: the names of the script functions its stimuli run, how far it
// senses, and its body colour. Reconstructed from SQZ.EXE 0x4483d0-0x448890 (its vtables are the StringList's: 0x49d50c, 0x49d4f4; the RTTI
// name "Character" belongs to the Possessions object, not to this one, and this class has no name of its own).
//
// A robo soul (SqueezilRoboSoul.h) turns what happens to it into a stimulus number (0 to 19, the order of kStimulusName: born, launched,
// landed, died on the ground or in the air, sensed a player / robo / goodie, lost the sense, bumped by one of them, each on the ground
// or in the air, and 19 "none") and asks its template for the script function of that stimulus. TrySetState chooses the function and
// leaves its name in the soul; RunPending runs it. Script functions change the behaviour in turn (SetRoboBehaviorFunction).
//
// The object is a StringList of 20 function names ("" for none) followed by the settings, 0x24 bytes. It is made by the level reader
// (@0044b66x) and kept by the place's level object.
#pragma once
#include "Container.h"
#include "GameString.h"

class IniReader;
struct SqueezilRoboSoul;

const int kStimulusCount = 20;
const int kNoStimulus = 19;             // STIM_NONE: nothing to run (kNoActivity in SqueezilRoboSoul.h)

struct CreatureTemplate : StringList {
    float senseRadius[3];               // 0x14 SenseRadiusP / R / G: how near a player / another robo / a goodie must be to be noticed
    int   bodyColor;                    // 0x20 the BodyColor key (ReadBodyColor)

    CreatureTemplate();                                         // @004483d0
    ~CreatureTemplate();                                        // @004484a0 (the function names are removed one by one, last first)
    // @00448510 (ret 8; the section is passed by address) Reads the section's STIM_* keys (the function names), SenseRadius* and BodyColor. Always 1.
    int  Read(const String &section, IniReader *ini);
    // @00448640 (ret 8) Sets a stimulus' function. The name "0" means none.
    void SetFunction(int stimulus, String name);
    // @004486f0 (ret 8) The stimulus has a function (and the soul is not dying, mode 6, unless it is stimulus 0): its name is left in the
    // soul and the soul remembers what it sensed. 0 when there is nothing to run.
    int  TrySetState(int stimulus, SqueezilRoboSoul *soul);
    // @004487f0 (ret 4) Runs the function the soul holds (a script function posted with the soul as the caller), if any.
    void RunPending(SqueezilRoboSoul *soul);
};
static_assert(sizeof(CreatureTemplate) == 0x24, "CreatureTemplate layout");

// @00448890 the stimulus with that name (case ignored), -1 for none
int FindStimulus(const char *name);
// @00448850 the behaviour (SqueezilRoboSoul mode 0-7: BEH_PACE, BEH_WANDER, BEH_SIMPLE_FLY, BEH_CHASE, BEH_SPIT, BEH_FLEE, BEH_DEATH, BEH_NONE) with that name, -1 for none
int FindBehaviourMode(const char *name);
