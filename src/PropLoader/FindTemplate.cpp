// SQZ.EXE 0x42c8b0: resolve a prop template name and its optional child path.
#include "compat41.h"
#include "Ini/IniFile.h"
#include <string.h>

class Prop;
typedef Table<String, Prop *, Array<Prop *>, 0> TemplatePropTable;
extern TemplatePropTable g_thirdProps; // 0x4c72c0
extern TemplatePropTable g_secondProps; // 0x4c7230
extern TemplatePropTable gGlobalProps; // 0x4c7208
void SplitColon(const char *name, StringList *words); // 0x42cdb0
Prop *WalkPropPath(const char *name, StringList *words, Prop *context, int mode); // 0x42cea0

Prop *FindPropTemplate(String name, Prop *context, int mode)
{
    const char *colon = strchr(name.c_str(), ':');
    StringList words;
    if (!colon)
        words.Insert(name, -1);
    else
        SplitColon(name.c_str(), &words);

    if (_stricmp(words.Get(0).c_str(), "parent") == 0 ||
        _stricmp(words.Get(0).c_str(), "this") == 0)
        return WalkPropPath(name.c_str(), &words, context, mode);
    else {
        Prop *found = 0;
        if (g_thirdProps.Contains(words.Get(0)))
            found = g_thirdProps.Get(words.Get(0));
        else if (g_secondProps.Contains(words.Get(0)))
            found = g_secondProps.Get(words.Get(0));
        else if (gGlobalProps.Contains(words.Get(0)))
            found = gGlobalProps.Get(words.Get(0));
        if (!found)
            return 0;
        words.Remove(0);
        return WalkPropPath(name.c_str(), &words, found, mode);
    }
}
