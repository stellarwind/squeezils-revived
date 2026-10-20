// 0x425870: read default sprite-decal settings, preserving unspecified values.
#include "World/WorldDecal.h"

struct IniReader;
struct SpriteDefaultsWorld {
    WorldDecalSettings ReadDecalSettings(IniReader *ini, const char *section,
        const char *key, WorldDecalSettings defaults); // 0x435c90
};
SpriteDefaultsWorld *GetSpriteDefaultsWorld(); // 0x4206a0
extern WorldDecalSettings gDefaultPropSprite;  // 0x4c7190
extern const char kSpriteDefaultsSection[];   // 0x4b9e60: "Defaults"
extern const char kDefaultPropSpriteKey[];    // 0x4bb8f4: "DefaultPropSprite"

void ReadDefaultPropSprite(IniReader *ini)
{
    gDefaultPropSprite = GetSpriteDefaultsWorld()->ReadDecalSettings(
        ini, kSpriteDefaultsSection, kDefaultPropSpriteKey, gDefaultPropSprite);
}
