// SQZ.EXE 0x44a1a0: release the current sound scope and its sample references.
#include "WaveSound/WaveSound.h"

struct SoundScope {
    void *vtable;
    int field04;
    int count;                             // key count, +0x8
    char pad0c[0x24 - 0xc];
    WaveSound **samples;                   // parallel values array, +0x24
    ~SoundScope();                         // 0x4684c0
};
struct SoundScopeStack {
    void *vtable;
    int count;
    void Remove(int index);                // 0x467f70
};
extern int g_audioOn;                       // 0x4bd95c
extern SoundScopeStack gSoundScopes;        // 0x4c7810
SoundScope *CurrentSoundScope();            // 0x44aad0

void ReleaseCurrentSoundScope()
{
    int index = 0;
    if (g_audioOn && gSoundScopes.count) {
        for (;;) {
            if (CurrentSoundScope()->count <= index)
                break;
            if (CurrentSoundScope()->samples[index]->useCount == 1)
                delete CurrentSoundScope()->samples[index];
            else
                --CurrentSoundScope()->samples[index]->useCount;
            ++index;
        }
        delete CurrentSoundScope();
        gSoundScopes.Remove(gSoundScopes.count - 1);
    }
}
