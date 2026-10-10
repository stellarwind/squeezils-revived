// CreatureTemplate: see CreatureTemplate.h. Reconstructed from SQZ.EXE 0x4483d0-0x448890.
#include "CreatureTemplate.h"
#include "SqueezilRoboSoul.h"
#include "SqueezilBody.h"
#include "IniReader.h"
#include "Simulation.h"
#include "Script.h"
#include "FrameProp.h"
#include "Original.h"
#include <string.h>

static_assert(offsetof(CreatureTemplate, bodyColor) == 0x20, "CreatureTemplate layout");

static const char *const kStimulusName[kStimulusCount] = {              // 0x49ec10 (the keys of the level file's section)
    "STIM_BORN", "STIM_LAUNCH", "STIM_LAND", "STIM_DIED_GND", "STIM_DIED_AIR",
    "STIM_SENSE_P_GND", "STIM_SENSE_P_AIR", "STIM_SENSE_R_GND", "STIM_SENSE_R_AIR", "STIM_SENSE_G_GND", "STIM_SENSE_G_AIR",
    "STIM_DESENSE_GND", "STIM_DESENSE_AIR",
    "STIM_BUMP_P_GND", "STIM_BUMP_P_AIR", "STIM_BUMP_R_GND", "STIM_BUMP_R_AIR", "STIM_BUMP_G_GND", "STIM_BUMP_G_AIR",
    "STIM_NONE"
};
static const char *const kBehaviourName[8] = {                          // 0x49ec60
    "BEH_PACE", "BEH_WANDER", "BEH_SIMPLE_FLY", "BEH_CHASE", "BEH_SPIT", "BEH_FLEE", "BEH_DEATH", "BEH_NONE"
};
static const char kNoFunction[] = "0";                                  // 0x4bca44: a function name of "0" is no function
static const char *const kSenseRadiusKey[3] = { "SenseRadiusP", "SenseRadiusR", "SenseRadiusG" };   // 0x4bd900, 0x4bd910, 0x4bd920

// @004483d0
CreatureTemplate::CreatureTemplate()
{
    for (int i = 0; i < kStimulusCount; i++) {
        String none("");
        Insert(none, -1);
    }
    senseRadius[0] = senseRadius[1] = senseRadius[2] = 0.0f;
    bodyColor = 0;                      // (left as the memory had it)
    UseOriginalVtables(static_cast<StringList *>(this));         // the original ends with the StringList's tables
}

// @004484a0
CreatureTemplate::~CreatureTemplate()
{
    for (int i = kStimulusCount - 1; i >= 0; i--)
        RemoveAt(i);
}

// @00448510
int CreatureTemplate::Read(const String &section, IniReader *ini)
{
    ini->SetSection(section.c_str());
    for (int i = 0; i < kStimulusCount; i++) {
        String function(ini->GetString(kStimulusName[i]));
        Replace(i, function);
    }
    senseRadius[2] = ini->GetFloat(kSenseRadiusKey[2], 0.0f);
    senseRadius[1] = ini->GetFloat(kSenseRadiusKey[1], 0.0f);
    senseRadius[0] = ini->GetFloat(kSenseRadiusKey[0], 0.0f);
    bodyColor = ReadBodyColor(ini);
    return 1;
}

// @00448640
void CreatureTemplate::SetFunction(int stimulus, String name)
{
    if (strcmp(name.c_str(), kNoFunction) == 0)
        *(char *)name.c_str() = 0;      // (the text is emptied in place)
    Replace(stimulus, name);
}

// @004486f0
int CreatureTemplate::TrySetState(int stimulus, SqueezilRoboSoul *soul)
{
    if (stimulus == kNoStimulus)
        return 0;
    if (data[stimulus].c_str()[0] == 0)
        return 0;
    if (soul->mode == 6 && stimulus != 0)
        return 0;
    soul->pendingFunction = data[stimulus].c_str();
    soul->Remember();
    return 1;
}

// @004487f0
void CreatureTemplate::RunPending(SqueezilRoboSoul *soul)
{
    if (!soul->pendingFunction)
        return;
    int id = ((LocationSimulation *)GetCurrentSimulation())->FindIdBySoul(soul);
    ScriptCaller caller;
    caller.unknown4 = 0;                // (the constructor leaves it as it was)
    ScriptCallerInitSoulOf(&caller, soul, id, NULL);
    PostFunction(soul->pendingFunction, &caller, 0, 0);
    soul->pendingFunction = NULL;
}

// @00448890
int FindStimulus(const char *name)
{
    for (int i = 0; i < kStimulusCount; i++)
        if (_stricmp(name, kStimulusName[i]) == 0)
            return i;
    return -1;
}

// @00448850
int FindBehaviourMode(const char *name)
{
    for (int i = 0; i < 8; i++)
        if (_stricmp(name, kBehaviourName[i]) == 0)
            return i;
    return -1;
}
