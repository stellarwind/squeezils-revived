// 0x435c90 (stdcall): override decal settings from the ini section, keys named <prefix>Colors, <prefix>Origin,
// <prefix>Size, <prefix>UseTransparency and <prefix>TransparentColor; missing keys keep the passed-in values.
#include <string.h>
#pragma intrinsic(strlen, strcpy, strcat)
#include "IniReader/IniReader.h"

struct DecalVec { float x, y, z; DecalVec(const DecalVec &o) : x(o.x), y(o.y), z(o.z) {} ~DecalVec(); };      // dtor 0x401660
struct DecalSettings {                                   // 0x2c bytes, dtor 0x425a30
    int colors;                  // 0x00
    DecalVec size;               // 0x04
    DecalVec origin;             // 0x10
    int useTransparency;         // 0x1c
    DecalVec transparentColor;   // 0x20
    ~DecalSettings();
};
extern const char kColors[], kOrigin[], kSize[], kUseTransparency[], kTransparentColor[];   // 0x4bc554 0x4bc54c 0x4bc544 0x4bc534 0x4bc520

DecalSettings __stdcall ReadDecalSettings(IniReader *ini, const char *section, const char *prefix, DecalSettings settings)
{
    char key[128];
    ini->SetSection(section);
    strcpy(key, prefix);
    strcat(key, kColors);
    if (strlen(ini->GetString(key)))
        settings.colors = ini->GetInt(key, 0);
    strcpy(key, prefix);
    strcat(key, kOrigin);
    if (strlen(ini->GetString(key)))
        ini->GetFloats(key, &settings.origin.x);
    strcpy(key, prefix);
    strcat(key, kSize);
    if (strlen(ini->GetString(key)))
        ini->GetFloats(key, &settings.size.x);
    strcpy(key, prefix);
    strcat(key, kUseTransparency);
    if (strlen(ini->GetString(key)))
        settings.useTransparency = ini->GetInt(key, 0);
    strcpy(key, prefix);
    strcat(key, kTransparentColor);
    if (strlen(ini->GetString(key)))
        ini->GetFloats(key, &settings.transparentColor.x);
    return settings;
}
