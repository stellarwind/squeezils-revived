// TurnTargetActivity::TurnTargetActivity (0x401510): the MoveActivity chain inlined, then the one-time [RoboPhysics] read.
#include "Activity/Activity.h"
#include "IniReader/IniReader.h"
extern int gSlowingRead;                 // 0x4b90e0
extern float gSlowingCutoff, gSlowingFactor, gMinSlowing;   // 0x4c63c0..c8

TurnTargetActivity::TurnTargetActivity(ABody *b, ASoul *s) : MoveActivity(b, s)
{
    if (!gSlowingRead) {
        IniReader ini;
        ini.SetSection("RoboPhysics");
        gSlowingCutoff = ini.GetFloat("TurnTargetSlowingCutoff", 0.7f);
        gSlowingFactor = ini.GetFloat("TurnTargetSlowingFactor", 0.5f);
        gMinSlowing = ini.GetFloat("TurnTargetMinSlowing", 0.035f);
        gSlowingRead = 1;
    }
}
