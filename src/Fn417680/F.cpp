// RegionSimulation::LoadLocations (SQZ.EXE 0x417680, ret 4): build a place for every [LocationList] entry of the region
// ini (key = location id, value = place name), then drop the links that point to places that do not exist.
#include "IniReader/IniReader.h"
#include "Container/ContainerSet.h"
#include "Container/ContainerTable.h"
#include <stdlib.h>
void DebugLog(const char *format, ...);                  // 0x419290

struct PlaceDescriptor {
    char pad[0x10];
    Array<int> *links;                                   // 0x10
};
typedef PrimitiveArray<int, 1> IntList;

struct RegionSimulation {
    char pad[0x30];
    Table<int, PlaceDescriptor *, Array<PlaceDescriptor *>, 8> places;   // 0x30

    PlaceDescriptor *MakePlace(IniReader *ini, const char *name, int id);   // 0x4178e0
    void LoadLocations(IniReader *ini);
};

void RegionSimulation::LoadLocations(IniReader *ini)
{
    ini->SetSection("LocationList");
    StringList *names = ini->GetKeys();
    int i;
    for (i = 0; i < names->count; i++) {
        ini->SetSection("LocationList");
        const char *key = names->Get(i).c_str();
        int id = atoi(key);
        const char *name = ini->GetString(key);
        DebugLog("Parsing location %d, named %s: ", id, name);
        PlaceDescriptor *place = MakePlace(ini, name, id);
        places.Insert(id, place, -1);
    }
    delete names;
    for (i = 0; i < places.keys.count; i++) {
        Array<int> *links = places.values.data[i]->links;
        IntList bad;
        int j;
        for (j = 0; j < links->count; j++) {
            if (!places.Contains(links->Get(j))) {
                bad.Insert(j, -1);
                DebugLog("Warning: place %d links to bad place %d!\n", places.keys.Get(i), links->Get(j));
            }
        }
        for (j = bad.count - 1; j >= 0; j--)
            links->Remove(j);
    }
}
