// SQZ.EXE 0x458590: load every base animation clip, then build the sub-clips (frame ranges of a named base clip); 0 on failure.
#include "IniReader/IniReader.h"
#include "MorphCycle/MorphCycle.h"
#include <string.h>

struct ClipDef {
    const char *name;           // 0x00
    int first, last, noLoop;    // 0x04
    int pad10;                  // 0x10
    float speed, period;        // 0x14
};
extern ClipDef g_baseClips[26];             // 0x49f860
extern ClipDef g_subClips[23];              // 0x49fb38
extern MorphCycle *g_baseCycles[26];        // 0x4bf070
extern MorphCycle *g_subCycles[23];         // 0x4bf0d8
extern const char kClipSection[];           // 0x4bf290
int LoadBaseClip(IniReader *ini, const char *name, ClipDef *def, MorphCycle **out);   // 0x458700 (cdecl)

int InitClips()
{
    IniReader ini;
    ini.SetSection(kClipSection);
    MorphCycle **slot = g_baseCycles;
    ClipDef *def = g_baseClips;
    do {
        if (!LoadBaseClip(&ini, def->name, def, slot))
            return 0;
        def++;
        slot++;
    } while (slot < &g_baseCycles[26]);
    MorphCycle **out = g_subCycles;
    ClipDef *sub = g_subClips;
    do {
        int index = 0;
        ClipDef *base = g_baseClips;
        do {
            if (!strcmp(base->name, sub->name))
                break;
            base++;
            index++;
        } while (base < g_subClips);
        *out = new MorphCycle(*g_baseCycles[index], sub->first, sub->last, sub->noLoop, sub->speed, sub->period);
        out++;
        sub++;
    } while (sub < &g_subClips[23]);
    return 1;
}
