// LandSimulation slot 1 (SQZ.EXE 0x41e490): the possessions table, the lent-id list, the player's possessions and the region names.
#include "IniReader/IniReader.h"
#include <string.h>
struct TableBase { TableBase(); ~TableBase(); virtual int Insert(const int &id, void *const &value, int at); char pad[0x28]; };           // ctor 0x4650b0, dtor 0x4651d0
struct BufObj { virtual void bo0(); };
struct PossTable : TableBase, BufObj {
    virtual void bo0();
};
struct IdList { char pad[0x14]; IdList(); };                                         // 0x45eef0
struct Possessions { char pad[8]; Possessions(); };                                  // 0x4488d0
void GameLog(const char *fmt);                                                       // 0x419290 (cdecl)
struct LandSimulation {
    int vtbl, pad;
    PossTable *table;                                                                // 0x08
    IdList *lentIds;                                                                 // 0x0c
    char **names;                                                                    // 0x10
    int regionCount;                                                                 // 0x14
    void Init();
};

void LandSimulation::Init()
{
    table = new PossTable;
    lentIds = new IdList;
    Possessions *player = new Possessions;
    int key = 0;
    table->Insert(key, player, -1);

    IniReader ini;
    ini.SetSection("RegionList");
    StringList *keys = ini.GetKeys();
    if (keys->count == 0) {
        GameLog("Error: no RegionList entry in sqz.ini!\n");
        return;
    }
    regionCount = keys->count;
    names = (char **)operator new(regionCount * 4 + 4);
    for (int i = 0; i < regionCount; i++) {
        names[i + 1] = (char *)operator new(0x50);
        strcpy(names[i + 1], ini.GetString(keys->Get(i).c_str()));
    }
    if (keys)
        delete keys;
}
