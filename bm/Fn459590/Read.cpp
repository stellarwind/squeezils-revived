// GoodieSoul::ReadSettings (SQZ.EXE 0x4597b0): the [GoodiePhysics] section of the INI file into the goodie settings.
#include "IniReader/IniReader.h"

struct Nine { int a, b, c, d, e, f, g, h, i; };
struct GoodieSettingsR {
    Nine d;                               // 0x00 copied from the defaults table
    float gravityFactor;                    // 0x24
    int bounceCount;                        // 0x28
    float bounceDamping;                    // 0x2c
    float collisionSpeedAmp;                // 0x30
    float angleOfSpray;                     // 0x34
    float initialSpraySpeed;                // 0x38
    float initialSprayPosOffset;            // 0x3c
};
extern GoodieSettingsR *g_goodieSettings;   // 0x4c7bb4
extern int g_goodieSettingsRead;            // 0x4bf354
extern Nine g_goodieDefaults;             // 0x4c7768
extern float g_goodieMass;                  // 0x4c7bb8
extern float g_goodieRebudPlane;            // 0x4c7bb0

struct GoodieReader { void ReadSettings(); };

void GoodieReader::ReadSettings()
{
    IniReader ini;
    ini.SetSection("GoodiePhysics");
    g_goodieSettingsRead = 1;
    GoodieSettingsR *s = g_goodieSettings;
    s->d.a = g_goodieDefaults.a;
    s->d.b = g_goodieDefaults.b;
    s->d.c = g_goodieDefaults.c;
    s->d.d = g_goodieDefaults.d;
    s->d.e = g_goodieDefaults.e;
    s->d.f = g_goodieDefaults.f;
    s->d.g = g_goodieDefaults.g;
    s->d.h = g_goodieDefaults.h;
    s->d.i = g_goodieDefaults.i;
    g_goodieSettings->gravityFactor = ini.GetFloat("GoodieGravityFactor", 1.0f);
    g_goodieSettings->bounceCount = ini.GetInt("BounceCount", 3);
    g_goodieSettings->bounceDamping = ini.GetFloat("BounceDamping", 0.5f);
    g_goodieSettings->collisionSpeedAmp = ini.GetFloat("CollisionSpeedAmp", 1.0f);
    g_goodieSettings->angleOfSpray = ini.GetFloat("AngleOfSpray", 45.0f);
    g_goodieSettings->initialSpraySpeed = ini.GetFloat("InitialSpraySpeed", 5.0f);
    g_goodieSettings->initialSprayPosOffset = ini.GetFloat("InitialSprayPosOffset", 1.0f);
    g_goodieMass = ini.GetFloat("GoodieMass", 0.0625f);
    g_goodieRebudPlane = ini.GetFloat("GoodieRebudPlane", -10000.0f);
}
