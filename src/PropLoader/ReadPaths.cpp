// 0x42c700: load and replace the named animation paths in an INI PathList.
#include "IniReader/IniReader.h"
#define MORPH_CYCLE_PATH_INTERFACE
#include "MorphCycle/MorphCycle.h"

struct PathRegistry {
    virtual int Insert(const String &name, MorphCycle *const &cycle, int hint);
    virtual int Contains(const String &name);
    MorphCycle *Get(const String &name); // 0x468ef0
};
extern const char kPathListSection[]; // 0x4bbe74

void ReadPaths(IniReader *ini, PathRegistry *paths)
{
    ini->SetSection(kPathListSection);
    StringList *keys = ini->GetKeys();
    for (int i = 0; i < keys->count; i++) {
        MorphCycle *path = new MorphCycle(0, 1.0f, -1.0f);
        ini->SetSection(kPathListSection);
        String name = ini->GetString(keys->Get(i).c_str());
        if (path->LoadPath(ini, name.c_str())) {
            if (paths->Contains(name))
                delete paths->Get(name);
            paths->Insert(name, path, -1);
        }
    }
    delete keys;
}
