// SQZ.EXE 0x449ee0 (cdecl): LoadScope - for every [AudioResources] entry of the ini, load "<name> = <file>.wav" as a new
// sample (relative names from the ini's directory), or make "<name> = <other>" share an already loaded sample.
#include <string.h>
#include <windows.h>
#include "IniReader/IniReader.h"

struct WaveSound { int refs; ~WaveSound(); };              // 0x448db0
struct SoundScope { virtual int Insert(const String &name, WaveSound *const &sound, int hint); };   // slot 0
SoundScope *CurrentSoundScope();                            // 0x44aad0
WaveSound *FindSound(const char *name);                     // 0x449c30 (cdecl)
int LoadSound(const char *name, const char *file);          // 0x449aa0 (cdecl)
void DebugLog(const char *format, ...);                     // 0x419290

void LoadScope(IniReader *ini)
{
    ini->SetSection("AudioResources");
    StringList *keys = ini->GetKeys();
    for (int i = 0; i < keys->count; i++) {
        const char *file = ini->GetString(keys->Get(i).c_str());
        if (strlen(file) > 5 && strcmp(file + strlen(file) - 4, ".wav") == 0) {
            if (!strchr(file, '\\'))
                SetCurrentDirectory(ini->GetDirectory().c_str());
            if (!LoadSound(keys->Get(i).c_str(), file))
                DebugLog("Warning: Sample %s not loaded properly, skipping.\n", file);
        } else {
            DebugLog("Shared effect entry: %s shares %s.\n", keys->Get(i).c_str(), file);
            WaveSound *sound = FindSound(file);
            if (!sound) {
                DebugLog("Error looking up shared audio resource %s\n", file);
                delete sound;
                return;
            }
            CurrentSoundScope()->Insert(keys->Get(i), sound, -1);
            sound->refs++;
        }
    }
    delete keys;
}
