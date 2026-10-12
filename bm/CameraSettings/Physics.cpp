// ReadPhysicsSettings (SQZ.EXE 0x446ed0): [Physics] Gravity, StaticFriction, KineticFriction into the shared PhysicalSettings.
#include "IniReader/IniReader.h"
struct PhysSettings { float SetGravity(float g); };               // 0x446dd0
extern PhysSettings gPhysSettings;                                 // 0x4c7768
extern float gKineticFriction;                                     // 0x4c7784
extern float gStaticFriction;                                      // 0x4c7788
extern int gPhysicsRead;                                           // 0x4bd6ac
void ReadPhysicsSettings()
{
    IniReader ini;
    ini.SetSection("Physics");
    gPhysSettings.SetGravity(ini.GetFloat("Gravity", -32.0f));
    gStaticFriction = ini.GetFloat("StaticFriction", 1.0f);
    gKineticFriction = ini.GetFloat("KineticFriction", 0.6f);
    gPhysicsRead = 1;
}
