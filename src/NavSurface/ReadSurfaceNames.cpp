// SQZ.EXE 0x40dad0: record names from [SurfaceList] and read their type sections.
#include "NavSurface/NavSurfaceList.h"
#include "IniReader/IniReader.h"

int DebugLogAll(const char *format, ...); // 0x419290

int NavSurfaceList::ReadSurfaceNames(IniReader *ini, Set<String, 11> *names)
{
    ini->SetSection("SurfaceList");
    Array<String> *keys = ini->GetKeys();
    Set<String, 11> &knownNames = *(Set<String, 11> *)((char *)this + 0x100);
    if (!knownNames.Contains(String("none")))
        knownNames.Insert(String("none"), -1);
    if (keys->count == 0) {
        delete keys;
        return 0;
    }
    for (int i=0; i<keys->count; ++i) {
        String name;
        name = String(ini->GetString(keys->Get(i).c_str()));
        if (knownNames.Contains(name))
            DebugLogAll("ERROR: surface name %s already present in surface type list!\n", name.c_str());
        knownNames.Insert(name, -1);
        names->Insert(name, -1);
        name.c_str();
        ReadSurfaceType(ini, name.c_str());
    }
    delete keys;
    return 1;
}
