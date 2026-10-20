// SQZ.EXE 0x44aae0: collect finished effects and schedule one-shot ambient sounds.
#include "compat41.h"
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdlib.h>
#include "String/String.h"

struct AmbientSample {
    int useCount, oneShot;
    float minimumDelay, maximumDelay, delay, elapsed;
    LPDIRECTSOUNDBUFFER buffer;
};
struct AmbientNameArray { String Get(int index); }; // 0x40ec10
struct EffectBufferArray { void Remove(int index); }; // 0x468120
struct AudioClock { unsigned SimRate(); }; // 0x41ec00
AudioClock *TheClock(); // 0x4206d0
AmbientSample *FindSound(const char *name); // 0x449c30
void DebugLog(const char *format, ...); // 0x419290
extern int g_audioOn; // 0x4bd95c
extern int gActiveSoundCount; // 0x4c77e4
extern LPDIRECTSOUNDBUFFER *gActiveSounds; // 0x4c77ec
extern EffectBufferArray gEffectBuffers; // 0x4c77e0
extern int g_streamCount; // 0x4c77d4
extern LPDIRECTSOUNDBUFFER *g_streams; // 0x4c77dc
extern AmbientNameArray gAmbientNames; // 0x4c77b0
extern const char kEffectStatusError[], kAmbientStatusError[], kAmbientStopError[]; // 0x4be158, 0x4be128, 0x4be10c

int AudioUpdate()
{
    if (!g_audioOn)
        return -1;
    DWORD status;
    for (int i = 0; i < gActiveSoundCount; ++i) {
        if (gActiveSounds[i]->GetStatus(&status) < 0) {
            DebugLog(kEffectStatusError);
            return -1;
        }
        if (status != 1 && status != 4) {
            status = gActiveSounds[i]->Release();
            gEffectBuffers.Remove(i);
        }
    }
    for (int j = 0; j < g_streamCount; ++j) {
        AmbientSample *sample = FindSound(gAmbientNames.Get(j).c_str());
        if (sample->oneShot == 1) {
            HRESULT result = g_streams[j]->GetStatus(&status);
            if (result < 0) {
                DebugLog(kAmbientStatusError);
                return -1;
            }
            if (status != 1) {
                if (*(int *)&sample->delay == (int)0xbf800000) {
                    // Retain the original's redundant error check.
                    if (result < 0)
                        DebugLog(kAmbientStopError, gAmbientNames.Get(j).c_str());
                    if (sample->minimumDelay == sample->maximumDelay)
                        sample->delay = sample->minimumDelay;
                    else {
                        int range = (int)((sample->maximumDelay - sample->minimumDelay) * 10.0f);
                        sample->delay = (rand() % range) * 0.1f + sample->minimumDelay;
                    }
                } else {
                    sample->elapsed += TheClock()->SimRate() * 0.001f;
                    if (sample->delay <= sample->elapsed) {
                        sample->buffer->Play(0, 0, 0);
                        sample->delay = -1.0f;
                        sample->elapsed = 0.0f;
                    }
                }
            }
        }
    }
    return gActiveSoundCount;
}
