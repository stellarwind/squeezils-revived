// SQZ.EXE 0x40d870: read surface definitions from input INIs, then the current INI.
#define SET_DEFAULT_CTOR_OUTOFLINE
#include <string.h>
#include "NavSurface/NavSurfaceList.h"
#include "IniReader/IniReader.h"

typedef Set<String, 11> SurfaceReadNames;
int DebugLogAll(const char *format, ...); // 0x419290

int NavSurfaceList::ReadSurfaceList(IniReader *ini)
{
    SurfaceReadNames &knownNames = *(SurfaceReadNames *)((char *)this + 0x100);
    Array<SurfaceReadNames *> &fileNames = *(Array<SurfaceReadNames *> *)((char *)this + 0x114);
    if (!knownNames.Contains(String("none")))
        knownNames.Insert(String("none"), -1);
    SurfaceReadNames *names = new SurfaceReadNames;
    fileNames.Insert(names, -1);
    ini->SetSection("InputFiles");
    Array<String> *keys = ini->GetKeys();
    for (int i=0; i<keys->count; ++i) {
        ini->SetSection("InputFiles");
        String file(ini->GetString(keys->Get(i).c_str()));
        if (strlen(file.c_str()) == 0) {
            DebugLogAll("ERROR: no input file numbered %d.\n", i);
        } else {
            IniReader other(file.c_str());
            ReadSurfaceNames(&other, names);
        }
    }
    delete keys;
    ReadSurfaceNames(ini, names);
    return 1;
}
