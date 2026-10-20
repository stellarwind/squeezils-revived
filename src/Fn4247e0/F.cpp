// SQZ.EXE 0x4247e0 (cdecl): open a new resource scope, then load every sub-ini listed in the [section] of this ini file.
#include <string.h>
#include "IniReader/IniReader.h"

struct SoundScope { SoundScope(); char body[0x28]; };    // 0x466de0
struct ScopeStack { void Insert(SoundScope *const &item, int at); };   // 0x466ad0
extern ScopeStack gSoundScopes;                          // 0x4c71c0
extern const char kSoundSection[];                       // 0x4b9d64
extern const char kLoadingScope[];                       // 0x4bb6e0
void LoadScope(IniReader *ini);                          // 0x4249a0 (cdecl)
int DebugLogAll(const char *format, ...);                  // 0x419290

void LoadResScopes(IniReader *ini)
{
    gSoundScopes.Insert(new SoundScope, -1);
    ini->SetSection(kSoundSection);
    StringList *keys = ini->GetKeys();
    int i = 0;
    for (; i < keys->count; i++) {
        String name(ini->GetString(keys->Get(i).c_str()));
        if (strlen(name.c_str())) {
            IniReader sub(name.c_str());
            DebugLogAll(kLoadingScope, sub.GetPath());
            LoadScope(&sub);
        }
    }
    if (keys)
        delete keys;
    LoadScope(ini);
}
