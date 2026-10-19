// 0x44add0: resume MIDI, active buffers and ambient streams.
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include "String/String.h"

struct WaveSound { int pad0; int oneShot; };
struct AmbientNameArray { String Get(int index); }; // 0x40ec10
extern AmbientNameArray gAmbientNames;             // 0x4c77b0
extern int g_audioOn;                              // 0x4bd95c
extern int gActiveSoundCount;                      // 0x4c77e4
extern LPDIRECTSOUNDBUFFER *gActiveSounds;          // 0x4c77ec
extern int g_streamCount;                          // 0x4c77d4
extern LPDIRECTSOUNDBUFFER *g_streams;              // 0x4c77dc
extern const char kAmbientRestartError[];          // 0x4be184
int ResumeMidi();                                 // 0x45abe0
WaveSound *FindSound(const char *name);             // 0x449c30
void GameLog(const char *format, ...);             // 0x419290

void ResumeSounds()
{
    int offset = 0;
    ResumeMidi();
    if (!g_audioOn)
        return;
    for (int i = 0; i < gActiveSoundCount; i++) {
        LPDIRECTSOUNDBUFFER buffer = *(LPDIRECTSOUNDBUFFER *)((char *)gActiveSounds + offset);
        if (buffer->Play(0, 0, 0) < 0)
            return;
        offset += 4;
    }
    for (int j = 0; j < g_streamCount; j++) {
        WaveSound *sound = FindSound(gAmbientNames.Get(j).c_str());
        if (g_streams[j]->Play(0, 0, sound->oneShot == 1 ? 0 : DSBPLAY_LOOPING) < 0)
            GameLog(kAmbientRestartError, gAmbientNames.Get(j).c_str());
    }
}
