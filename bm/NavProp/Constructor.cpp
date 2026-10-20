// NavProp constructor, 0x4296d0: copy navigation bindings after building the cycle prop.
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"
#include "Ini/IniFile.h"
struct NavCtorBlock;
struct NavCtorCycle {
    virtual ~NavCtorCycle();
    char data[0xf4 - 4];
    NavCtorCycle(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        NavCtorBlock *names, void *cycle, NavCtorBlock *messages, NavCtorBlock *start,
        float stepPeriod, int keepRemainder, float clipStart); // 0x4288b0
};
struct NavCtorBindings {
    IniValueTable table;
    String KeyAt(int index); // 0x40ee50
    String ValueAt(int index); // 0x40f2a0
};
struct NavCtor : NavCtorCycle {
    int surface, instance;
    IniValueTable properties;
    float sizeSquared;
    NavCtor(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
        NavCtorBlock *names, void *cycle, NavCtorBlock *messages, NavCtorBlock *start,
        float stepPeriod, int keepRemainder, float clipStart, int surfaceId, NavCtorBindings *bindings);
};
NavCtor::NavCtor(Vector3f position, Vector3f scale, Vector3f direction, Vector3f up,
    NavCtorBlock *names, void *cycle, NavCtorBlock *messages, NavCtorBlock *start,
    float stepPeriod, int keepRemainder, float clipStart, int surfaceId, NavCtorBindings *bindings)
    : NavCtorCycle(position, scale, direction, up, names, cycle, messages, start, stepPeriod, keepRemainder, clipStart)
{
    surface = surfaceId;
    for (int i = 0; i < bindings->table.keys.count; ++i) {
        String key = bindings->KeyAt(i);
        String value = bindings->ValueAt(i);
        properties.Insert(key, value, -1);
    }
}
