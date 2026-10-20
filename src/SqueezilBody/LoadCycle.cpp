// 0x458700: load a named animation cycle and clear the output on failure.
#include <string.h>
#define MORPH_CYCLE_LOAD_INTERFACE
#include "MorphCycle/MorphCycle.h"
#include "IniReader/IniReader.h"

struct CycleLoadSettings {
    char pad[0x0c];
    int noLoop;
    int pad10;
    float speed, period;
};
void GameLog(const char *format, ...);     // 0x419290
extern const char kLoadingCycle[];        // 0x4bf2b8
extern const char kMissingCycleFile[];    // 0x4bf21c
extern const char kCycleLoadError[];      // 0x4bf298
extern const char kCycleLoadDone[];       // 0x4b9dbc

int LoadBodyCycle(IniReader *ini, const char *name,
    const CycleLoadSettings *settings, MorphCycle **out)
{
    GameLog(kLoadingCycle, name);
    *out = new MorphCycle(settings->noLoop, settings->speed, settings->period);
    const char *filename = ini->GetString(name);
    if (strlen(filename) == 0) {
        GameLog(kMissingCycleFile);
        delete *out;
        *out = 0;
        return 0;
    }
    if (!(*out)->Load(filename)) {
        GameLog(kCycleLoadError, filename);
        delete *out;
        *out = 0;
        return 0;
    }
    GameLog(kCycleLoadDone);
    return 1;
}
