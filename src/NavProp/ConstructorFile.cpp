#include "Runtime/DynamicCast.h"
// NavProp constructor, 0x429380 (ret 0x5c): build the cycle prop, load its navigation surface from a file through
// the place's surface list, take the surface's property names (empty values) and instance, then copy the bindings.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "Ini/IniFile.h"
struct NavFileBlock;
struct NavFileCycle {
    virtual ~NavFileCycle();
    char data[0xf4 - 4];
    NavFileCycle(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        NavFileBlock *names, void *cycle, NavFileBlock *messages, NavFileBlock *start,
        float stepPeriod, int keepRemainder, float clipStart); // 0x4288b0
};
struct NavFileBindings {
    IniValueTable table;
    String KeyAt(int index); // 0x40ee50
    String ValueAt(int index); // 0x40f2a0
};
struct NavFileNames {
    virtual String Get(int index); // slot 0
    int count;                     // 0x04
};
struct NavFileInfo {
    char pad[4];
    int instance;                  // 0x04
    char pad8[0x14 - 8];
    NavFileNames names;            // 0x14
};
struct NavFileSurface;
struct NavFileSurfaces {
    NavFileSurface *Load(const char *a, const char *b, const char *c); // 0x40fc10
    NavFileInfo *Info(NavFileSurface *surface);                        // 0x412c10
};
struct NavFilePlace { NavFileSurfaces *GetSurfaces(); }; // 0x439b60
void *GetSimulation();                                    // 0x420690 (cdecl)
extern const char kWorldSimulationType[], kWorldPlaceType[]; // 0x4ba1d0, 0x4bae38
extern const char kEmptyValue[];                          // 0x4b9cf4
struct NavFileProp : NavFileCycle {
    NavFileSurface *surface;       // 0xf4
    int padf8;
    IniValueTable properties;      // 0xfc
    int instance;                  // 0x12c
    NavFileProp(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        NavFileBlock *names, void *cycle, NavFileBlock *messages, NavFileBlock *start,
        float stepPeriod, int keepRemainder, float clipStart,
        const char *a, const char *b, const char *c, NavFileBindings *bindings);
};
NavFileProp::NavFileProp(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
    NavFileBlock *names, void *cycle, NavFileBlock *messages, NavFileBlock *start,
    float stepPeriod, int keepRemainder, float clipStart,
    const char *a, const char *b, const char *c, NavFileBindings *bindings)
    : NavFileCycle(position, scale, direction, up, names, cycle, messages, start, stepPeriod, keepRemainder, clipStart)
{
    surface = ((NavFilePlace *)SqzRTDynamicCast(GetSimulation(), 0, kWorldSimulationType, kWorldPlaceType, 0))
        ->GetSurfaces()->Load(a, b, c);
    NavFileInfo *info = ((NavFilePlace *)SqzRTDynamicCast(GetSimulation(), 0, kWorldSimulationType, kWorldPlaceType, 0))
        ->GetSurfaces()->Info(surface);
    int i;
    for (i = 0; i < info->names.count; ++i)
        properties.Insert(info->names.Get(i), String(kEmptyValue), -1);
    instance = info->instance;
    for (i = 0; i < bindings->table.keys.count; ++i) {
        String key = bindings->KeyAt(i);
        String value = bindings->ValueAt(i);
        properties.Insert(key, value, -1);
    }
}
