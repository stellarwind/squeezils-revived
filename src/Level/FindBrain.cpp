// 0x44e1b0: find a named brain, falling back to the first registered entry.
#include "String/String.h"

struct LevelBrainTable {
    virtual void slot0();
    virtual int Contains(const String &name);
    char pad[0x20];
    void **values;                              // +0x24
    String KeyAt(int index);                    // 0x44de90
    void *Get(const String &name);              // 0x46a210
};
struct BrainLookupLevel {
    char pad[0x2cc];
    LevelBrainTable brains;
    void *FindBrain(String name);
};
extern const char kMissingBrainWarning[];        // 0x4be66c
int DebugLogAll(const char *format, ...);           // 0x419290

void *BrainLookupLevel::FindBrain(String name)
{
    if (!brains.Contains(name)) {
        String attempted = name;
        DebugLogAll(kMissingBrainWarning, attempted.c_str(), brains.KeyAt(0).c_str());
        return brains.values[0];
    }
    return brains.Get(name);
}
