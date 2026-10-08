// TextureDefaults: the defaults the texture and sprite loaders start from. Reconstructed from SQZ.EXE 0x424e20 (called once at
// start-up, from 0x4482ef). The loaders themselves (0x425070 and the resource list readers around it) still run original code.
#pragma once

// The pixel format textures are loaded in: the renderer's own format (Renderer::GetPixelFormat), unless the [Defaults] section of
// the ini file overrides all six values with the OverrideTexture* keys. Lives at 0x4c7168 as six dwords.
struct TexturePixelFormat {
    int depth;          // 0x00 bits per pixel (OverrideTextureDepth)
    int isPalettized;   // 0x04 (OverrideTextureIsPalettized)
    int redMask;        // 0x08 (OverrideTextureRedMask)
    int greenMask;      // 0x0c (OverrideTextureGreenMask)
    int blueMask;       // 0x10 (OverrideTextureBlueMask)
    int alphaMask;      // 0x14 (OverrideTextureAlphaMask)
};
static_assert(sizeof(TexturePixelFormat) == 24, "TexturePixelFormat layout");

// @00424e20 Resets the default decal settings (the record at 0x4c7190, which the "DefaultPropSprite" keys later override: see
// World::ReadDecalSettings) and sets the texture pixel format.
void InitTextureDefaults();
