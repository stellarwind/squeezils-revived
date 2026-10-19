// SQZ.EXE 0x4241d0: initialize geometry-loader callbacks and perspective settings.
#include "IniReader/IniReader.h"
#include "Globals/Globals.h"
#include "Misc/Misc.h"
#include <string.h>

struct GeometryRenderer {
    char pad00[0x984];
    int hardware;
};
extern GeometryRenderer *gRenderer;                     // 0x4c75dc
extern char gGeometrySavedDirectory[128];               // 0x4c70e8
extern int gUsePerspectiveCorrection;                   // 0x4bb5f0
inline GeometryRenderer *GeometryRendererInstance() { return gRenderer; }

void InitGeometryLoading()
{
    strcpy(gGeometrySavedDirectory, kEmptyString);
    WrlSetLogCallback((void *)SqgErrorLog);
    WrlSetErrorCallback((void *)SqgErrorLog);
    IniReader ini;
    ini.SetSection("Render");
    gUsePerspectiveCorrection = ini.GetInt("UsePerspectiveCorrection", 0);
    if (GeometryRendererInstance()->hardware)
        gUsePerspectiveCorrection = 1;
}
