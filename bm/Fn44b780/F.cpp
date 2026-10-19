// SQZ.EXE 0x44b780: load every sub-ini listed under the section, then this ini itself; always 1.
#include <string.h>
#include "IniReader/IniReader.h"

extern const char kSoundSection[];                       // 0x4b9d64
extern const char kLoadingSub[], kBadKey[];              // 0x4be220 0x4b9d40
void DebugLog(const char *format, ...);                  // 0x419290

struct SubLoader {
    int Load(IniReader *ini);                            // 0x44b5b0
    int LoadAll();
};

int SubLoader::LoadAll()
{
    IniReader ini;
    ini.SetSection(kSoundSection);
    StringList *keys = ini.GetKeys();
    int i = 0;
    for (; i < keys->count; i++) {
        String name;
        ini.SetSection(kSoundSection);
        name = String(ini.GetString(keys->Get(i).c_str()));
        if (strlen(name.c_str())) {
            IniReader sub(name.c_str());
            DebugLog(kLoadingSub, sub.GetPath());
            Load(&sub);
        } else
            DebugLog(kBadKey, i);
    }
    Load(&ini);
    if (keys)
        delete keys;
    return 1;
}
