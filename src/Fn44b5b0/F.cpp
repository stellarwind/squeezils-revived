// SQZ.EXE 0x44b5b0 (ret 4): read the [RoboBrainList] of an ini file and load every brain it names into this set; always 1.
#include "IniReader/IniReader.h"

struct RoboBrain {
    RoboBrain();                                         // 0x4483d0
    int Load(const String &name, IniReader *ini);        // 0x448510
    ~RoboBrain();                                        // 0x4484a0
    char body[0x24];
};
struct BrainTable {
    virtual int Insert(const String &name, RoboBrain *const &brain, int hint);   // slot 0
};
extern const char kBrainList[];                          // 0x4be210
extern const char kBrainError[];                         // 0x4be1ec
void DebugLog(const char *format, ...);                  // 0x419290

struct BrainSet {
    char pad[0x2cc];
    BrainTable table;                                    // 0x2cc
    int LoadList(IniReader *ini);
};

int BrainSet::LoadList(IniReader *ini)
{
    ini->SetSection(kBrainList);
    StringList *keys = ini->GetKeys();
    int i = 0;
    for (; i < keys->count; i++) {
        String name;
        name = String(ini->GetString(keys->Get(i).c_str()));
        RoboBrain *brain = new RoboBrain;
        if (!brain->Load(name, ini)) {
            DebugLog(kBrainError, *(char **)&name);
            delete brain;
        } else {
            table.Insert(name, brain, -1);
            ini->SetSection(kBrainList);
        }
    }
    if (keys)
        delete keys;
    return 1;
}
