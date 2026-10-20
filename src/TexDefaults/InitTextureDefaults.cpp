// 0x424e20: reset the default decal and take the texture pixel format from the renderer, optionally overridden from the ini.
#include <string.h>
#include "Vector3f/Vector3f.h"
#include "IniReader/IniReader.h"
#include "Renderer/Renderer.h"

// SIZE 0x2c
struct DecalSettings {
    int colors;                  // 0x00
    Vector3f size;               // 0x04
    Vector3f origin;             // 0x10
    int useTransparency;         // 0x1c
    Vector3f transparentColor;   // 0x20
};
// SIZE 0x18
struct PixelFormat {
    int depth;          // 0x00
    int isPalettized;   // 0x04
    int redMask;        // 0x08
    int greenMask;      // 0x0c
    int blueMask;       // 0x10
    int alphaMask;      // 0x14
};
extern Renderer *g_renderer;                             // 0x4c75dc
extern DecalSettings g_defaultDecal;                     // 0x4c7190
extern PixelFormat g_texturePixelFormat;                 // 0x4c7168

void InitTextureDefaults()
{
    g_defaultDecal.colors = 0;
    g_defaultDecal.size = Vector3f(1.0f, 1.0f, 1.0f);
    g_defaultDecal.origin = Vector3f(0.0f, 0.0f, 0.0f);
    g_defaultDecal.useTransparency = 0;
    g_defaultDecal.transparentColor = Vector3f(0.0f, 0.0f, 0.0f);

    int format[6];
    memcpy(&g_texturePixelFormat, g_renderer->GetPixelFormat(format), sizeof g_texturePixelFormat);

    IniReader ini;
    int depth = ini.GetInt("OverrideTextureDepth", -666);
    int isPalettized = ini.GetInt("OverrideTextureIsPalettized", -666);
    int redMask = ini.GetInt("OverrideTextureRedMask", -666);
    int greenMask = ini.GetInt("OverrideTextureGreenMask", -666);
    int blueMask = ini.GetInt("OverrideTextureBlueMask", -666);
    int alphaMask = ini.GetInt("OverrideTextureAlphaMask", -666);
    if (depth != -666 && isPalettized != -666 && redMask != -666 &&
        greenMask != -666 && blueMask != -666 && alphaMask != -666) {
        g_texturePixelFormat.depth = depth;
        g_texturePixelFormat.isPalettized = isPalettized;
        g_texturePixelFormat.redMask = redMask;
        g_texturePixelFormat.greenMask = greenMask;
        g_texturePixelFormat.blueMask = blueMask;
        g_texturePixelFormat.alphaMask = alphaMask;
    }
}
