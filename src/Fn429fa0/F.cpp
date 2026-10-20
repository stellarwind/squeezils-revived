// 0x429fa0 (cdecl): ReadPropFiles - read the paths, then the props, of every ini named in [InputFiles] and of the ini
// itself; the splash screen advances about ten times while the props load.
#include <string.h>
#include "IniReader/IniReader.h"
struct PropTable;
struct PathRegistry;
void ReadPaths(IniReader *ini, PathRegistry *paths);        // 0x42c700 (cdecl)
void ReadProps(IniReader *ini, PropTable *props);           // 0x42a290 (cdecl)
int DebugLogAll(const char *format, ...);                     // 0x419290
struct Splash { virtual void SetProgress(float p); virtual void Step(); };
extern Splash *g_splash;                                    // 0x4c75e8

void ReadPropFiles(IniReader *ini, PropTable *props, PathRegistry *paths)
{
    ini->SetSection("InputFiles");
    StringList *files = ini->GetKeys();
    int i;
    for (i = 0; i < files->count; i++) {
        ini->SetSection("InputFiles");
        String name(ini->GetString(files->Get(i).c_str()));
        if (strlen(name.c_str())) {
            IniReader file(name.c_str());
            DebugLogAll("Reading paths from %s...\n", file.GetPath());
            ReadPaths(&file, paths);
        } else {
            DebugLogAll("ERROR: no input file numbered %d.\n", i);
        }
    }
    ReadPaths(ini, paths);
    int loaded = 0;
    int every;
    if (files->count < 10)
        every = 1;
    else
        every = files->count / 10;
    for (i = 0; i < files->count; i++) {
        ini->SetSection("InputFiles");
        String name(ini->GetString(files->Get(i).c_str()));
        if (strlen(name.c_str())) {
            IniReader file(name.c_str());
            DebugLogAll("Reading props from %s...\n", file.GetPath());
            ReadProps(&file, props);
            if (loaded % every == 0 || every == 1)
                g_splash->Step();
            loaded++;
        } else {
            DebugLogAll("ERROR: no input file numbered %d.\n", i);
        }
    }
    delete files;
    ReadProps(ini, props);
}
