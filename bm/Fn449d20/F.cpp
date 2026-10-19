// SQZ.EXE 0x449d20 (cdecl): open a new sound scope, load the scope named by the ini file, then every sub-ini listed in its [section]; no-op without audio.
#include <string.h>
#include "IniReader/IniReader.h"

struct SoundScope { SoundScope(); char body[0x28]; };    // 0x468210
struct ScopeStack { void Insert(SoundScope **item, int at); };   // 0x467f00
extern ScopeStack gSoundScopes;                          // 0x4c7810
extern int g_audioOn;                                    // 0x4bd95c
extern const char kSoundSection[];                       // 0x4b9d64
extern const char kLoadingScope[];                       // 0x4bdeec
void LoadScope(IniReader *ini);                          // 0x449ee0 (cdecl)
void DebugLog(const char *format, ...);                  // 0x419290

void LoadSoundScopes(IniReader *ini)
{
    if (!g_audioOn)
        return;
    {
        SoundScope *scope = new SoundScope;
        gSoundScopes.Insert(&scope, -1);
    }
    LoadScope(ini);
    ini->SetSection(kSoundSection);
    StringList *keys = ini->GetKeys();
    for (int i = 0; i < keys->count; i++) {
        String name(ini->GetString(keys->Get(i).c_str()));
        if (strlen(name.c_str())) {
            IniReader sub(name.c_str());
            DebugLog(kLoadingScope, sub.GetPath());
            LoadScope(&sub);
        }
    }
    if (keys)
        delete keys;
}
