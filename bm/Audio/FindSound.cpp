// SQZ.EXE 0x449c30: search sound scopes from newest to oldest for a named sample.
#include "String/String.h"

struct WaveSound;
struct SoundLookupScope {
    virtual void slot0();
    virtual int Contains(const String &name);
    WaveSound *Get(const String &name);                 // 0x468630
};
struct SoundLookupStack {
    void *vtable;
    int count, capacity;
    SoundLookupScope **data;
    SoundLookupScope *At(int index) { return data[index]; }
};
extern SoundLookupStack gSoundScopes;                    // 0x4c7810

WaveSound *FindSound(const char *name)
{
    for (int index = gSoundScopes.count - 1; index >= 0; --index) {
        if (gSoundScopes.At(index)->Contains(String(name)))
            return gSoundScopes.At(index)->Get(String(name));
    }
    return 0;
}
