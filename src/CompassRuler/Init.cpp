// SQZ.EXE 0x414490: create the compass pictures and map their region names.
#include "CompassC.h"
#include "Bitmap2/Bitmap2.h"
#include "IniReader/IniReader.h"
#include <string.h>
#include <stdlib.h>
#pragma intrinsic(strcpy, strcmp)

extern Renderer *g_renderer;                    // 0x4c75dc
extern char g_iniPath[];                        // 0x4c74d8
extern const char gText[];                      // 0x4b9488
extern const char kCompassSection[], kCompassRiver[], kCompassCoast[], kCompassDesert[],
    kCompassPine[], kCompassJungle[], kCompassSpacer[], kCompassRegionList[],
    kCompassPineRegion[], kCompassJungleRegion[], kCompassDesertRegion[], kCompassCoastRegion[];
    // 0x4ba27c, 0x4ba274, 0x4ba26c, 0x4ba264, 0x4ba25c, 0x4ba254, 0x4ba24c, 0x4ba240,
    // 0x4ba238, 0x4ba230, 0x4ba228, 0x4ba220

int CompassC::Init()
{
    IniReader ini;
    ini.SetSection(kCompassSection);
    strcpy(name[0], ini.GetString(kCompassRiver));
    picture[0] = (StripBitmap *)new Bitmap(g_renderer);
    picture[1] = (StripBitmap *)new Bitmap(g_renderer);
    strcpy(name[1], ini.GetString(kCompassCoast));
    picture[2] = (StripBitmap *)new Bitmap(g_renderer);
    picture[3] = (StripBitmap *)new Bitmap(g_renderer);
    strcpy(name[2], ini.GetString(kCompassDesert));
    picture[4] = (StripBitmap *)new Bitmap(g_renderer);
    picture[5] = (StripBitmap *)new Bitmap(g_renderer);
    strcpy(name[5], ini.GetString(kCompassPine));
    picture[6] = (StripBitmap *)new Bitmap(g_renderer);
    picture[7] = (StripBitmap *)new Bitmap(g_renderer);
    strcpy(name[3], ini.GetString(kCompassJungle));
    picture[8] = (StripBitmap *)new Bitmap(g_renderer);
    picture[9] = (StripBitmap *)new Bitmap(g_renderer);
    strcpy(name[4], ini.GetString(kCompassSpacer));
    picture[10] = (StripBitmap *)new Bitmap(g_renderer);
    picture[11] = (StripBitmap *)new Bitmap(g_renderer);

    char keys[100], value[20];
    int at = 0;
    GetPrivateProfileStringA(kCompassRegionList, 0, gText, keys, 100, g_iniPath);
    char current = keys[0];
    while (current) {
        char *key = keys + at;
        GetPrivateProfileStringA(kCompassRegionList, key, gText, value, 20, g_iniPath);
        if (!strcmp(value + 8, kCompassPineRegion)) pineRegion = atoi(key) - 1;
        if (!strcmp(value + 8, kCompassJungleRegion)) jungleRegion = atoi(key) - 1;
        if (!strcmp(value + 8, kCompassDesertRegion)) desertRegion = atoi(key) - 1;
        if (!strcmp(value + 8, kCompassCoastRegion)) coastRegion = atoi(key) - 1;
        current = *key;
        while (current) current = keys[++at];
        ++at;
        current = keys[at];
    }
    UpdateSpritesReal();
    x = 261;
    y = 29;
    return 1;
}
