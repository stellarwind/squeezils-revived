// SQZ.EXE 0x40dd30: parse a type's navigation flags and scripts, replacing its old entry.
#include <string.h>
#include "NavSurface/NavSurfaceList.h"
#include "IniReader/IniReader.h"
#include "NavSurface/SurfaceTypeDescription.h"

typedef Table<String, SurfaceTypeDescription *, Array<SurfaceTypeDescription *>, 0> SurfaceTypeTable;
void GameLog(const char *format, ...); // 0x419290

int NavSurfaceList::ReadSurfaceType(IniReader *ini, const char *name)
{
    String saved(ini->section);
    ini->SetSection(name);
    ini->GetString("Class");
    const char *navigable = ini->GetString("Navigable");
    int walkable=1, bounce=0, slippery=0, flyThrough=0;
    if (strcmp(navigable, "false") == 0)
        walkable=0;
    else if (strcmp(navigable, "true") != 0) {
        if (strcmp(navigable, "bounce") == 0)
            bounce=1;
        else if (strcmp(navigable, "slippery") == 0)
            slippery=1;
        else if (strcmp(navigable, "flyThrough") == 0)
            flyThrough=1;
        else
            walkable=1;
    }
    float rebud = ini->GetFloat("RebudGoodies", 0.1f);
    String activate(ini->GetString("Activate"));
    String deactivate(ini->GetString("Deactivate"));
    SurfaceTypeTable &types = *(SurfaceTypeTable *)((char *)this + 0x16c);
    if (types.Contains(String(name)))
        delete types.Get(String(name));
    SurfaceTypeDescription *description = new SurfaceTypeDescription(
        walkable, bounce, slippery, flyThrough, rebud, activate.c_str(), deactivate.c_str());
    types.Insert(String(name), description, -1);
    GameLog("OK.\n");
    ini->SetSection(saved.c_str());
    return 1;
}
