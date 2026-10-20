// TextureDefaults: see TextureDefaults.h. Reconstructed from SQZ.EXE 0x424e20.
#include <string.h>
#include "TextureDefaults.h"
#include "World.h"
#include "IniReader.h"
#include "Renderer.h"
#include "Original.h"

static DecalSettings *const kDefaultDecal = (DecalSettings *)0x4c7190;          // the "DefaultPropSprite" record (World.h)
static TexturePixelFormat *const kTexturePixelFormat = (TexturePixelFormat *)0x4c7168;

// An unset override: the ini reader is asked for each value with this default, and the format is only replaced when none of
// the six comes back as this.
static const int kNoOverride = -666;

// @00424e20
void InitTextureDefaults()
{
    DecalSettings *decal = kDefaultDecal;
    decal->colors = 0;
    decal->size[0] = 1.0f;
    decal->size[1] = 1.0f;
    decal->size[2] = 1.0f;
    decal->origin[0] = 0.0f;
    decal->origin[1] = 0.0f;
    decal->origin[2] = 0.0f;
    decal->useTransparency = 0;
    decal->transparentColor[0] = 0.0f;
    decal->transparentColor[1] = 0.0f;
    decal->transparentColor[2] = 0.0f;

    int format[6];
    memcpy(kTexturePixelFormat, GetRenderer()->GetPixelFormat(format), sizeof *kTexturePixelFormat);

    IniReader ini;
    int depth = ini.GetInt("OverrideTextureDepth", kNoOverride);
    int isPalettized = ini.GetInt("OverrideTextureIsPalettized", kNoOverride);
    int redMask = ini.GetInt("OverrideTextureRedMask", kNoOverride);
    int greenMask = ini.GetInt("OverrideTextureGreenMask", kNoOverride);
    int blueMask = ini.GetInt("OverrideTextureBlueMask", kNoOverride);
    int alphaMask = ini.GetInt("OverrideTextureAlphaMask", kNoOverride);
    if (depth != kNoOverride && isPalettized != kNoOverride && redMask != kNoOverride &&
        greenMask != kNoOverride && blueMask != kNoOverride && alphaMask != kNoOverride) {
        kTexturePixelFormat->depth = depth;
        kTexturePixelFormat->isPalettized = isPalettized;
        kTexturePixelFormat->redMask = redMask;
        kTexturePixelFormat->greenMask = greenMask;
        kTexturePixelFormat->blueMask = blueMask;
        kTexturePixelFormat->alphaMask = alphaMask;
    }
}
