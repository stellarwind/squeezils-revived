// 0x425500 (cdecl): load every entry of [TextureResources]. A value ending in one of the two image extensions is
// a file, loaded from the ini's directory when it has no path; any other value names an entry it shares.
#include <windows.h>
#include <string.h>
#pragma intrinsic(strcmp, strlen, strcpy)
#include "IniReader/IniReader.h"

struct Texture { virtual long __stdcall q0(); virtual unsigned long __stdcall AddRef(); virtual unsigned long __stdcall Release(); };
struct TexTableT {
    virtual int Insert(const String &name, Texture *const &tex, int hint);   // slot 0
};
struct Splash {
    virtual void v0();
    virtual void Step();                                 // slot 1
};
extern Splash *g_splash;                                 // 0x4c75e8
extern char gSavedDirectory[];                           // 0x4c70e8
extern const char kNoText[];                              // 0x4b9488
extern const char kTextureResources[];                   // 0x4bb8e0
extern const char kExtA[], kExtB[];                      // 0x4bb62c 0x4bb834
extern const char kSharedEntry[];                        // 0x4bb8b8
extern const char kNoEntryToShare[];                     // 0x4bb708
int DebugLogAll(const char *format, ...);                  // 0x419290
Texture *LoadTexture(const char *name, const char *file);   // 0x425070
Texture *FindTexture(const char *name);                  // 0x425220
TexTableT *TopTable();                                   // 0x425bf0
void ReadDefaultPropSprite(IniReader *ini);              // 0x425870

void ReadTextureList(IniReader *ini)
{
    ini->SetSection(kTextureResources);
    StringList *keys = ini->GetKeys();
    int i = 0;
    for (; i < keys->count; i++) {
        const char *value = ini->GetString(keys->Get(i).c_str());
        if (strlen(value) > 5 && (!strcmp(value + strlen(value) - 4, kExtA) || !strcmp(value + strlen(value) - 4, kExtB))) {
            int noPath = strchr(value, '\\') == 0;
            String unused;
            if (noPath) {
                if (!GetCurrentDirectory(0x80, gSavedDirectory))
                    noPath = 0;
                SetCurrentDirectory(ini->GetDirectory().c_str());
            }
            if (i % 20 == 0)
                g_splash->Step();
            LoadTexture(keys->Get(i).c_str(), value);
            if (noPath) {
                SetCurrentDirectory(gSavedDirectory);
                strcpy(gSavedDirectory, kNoText);
            }
        } else {
            const char *name = keys->Get(i).c_str();
            DebugLogAll(kSharedEntry, name, value);
            Texture *tex = FindTexture(value);
            if (!tex) {
                DebugLogAll(kNoEntryToShare, value);
            } else {
                TopTable()->Insert(keys->Get(i), tex, -1);
                tex->AddRef();
            }
        }
    }
    if (keys)
        delete keys;
    ReadDefaultPropSprite(ini);
}
