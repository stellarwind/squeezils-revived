// Sound volume setters, SQZ.EXE 0x44a940 / 0x44aa10 / 0x44aab0, and 0x44aad0 (last stream).
#include <windows.h>
#include <dsound.h>

extern int g_audioOn;                 // 0x4bd95c
extern int g_streamOn;                // 0x4bd960
extern float g_musicVolume;           // 0x4c77cc
extern int g_musicDb;                 // 0x4c77f0
extern int g_sfxCount;                // 0x4c77e4
extern LPDIRECTSOUNDBUFFER *g_sfx;    // 0x4c77ec
extern float g_sfxVolume;             // 0x4c77c8
extern int g_sfxDb;                   // 0x4c77a4
extern int g_streamCount;             // 0x4c77d4
extern LPDIRECTSOUNDBUFFER *g_streams;   // 0x4c77dc
extern float g_masterVolume;          // 0x4c77a0
extern const float kDbScale;          // 0x49eccc
void DebugLog(const char *format, ...);   // 0x419290
void SetMasterVolume(float v);        // 0x45ac40

void SetMusicVolume(float v)
{
    if (g_audioOn) {
        g_musicVolume = v;
        g_musicDb = (int)(v * 10000.0f - 10000.0f);
        DebugLog("setting music volume %d\n", g_musicDb);
        for (int i = 0; g_sfxCount > i; i++) {
            if (g_sfx[i]->SetVolume(g_musicDb) < 0)
                DebugLog("could not set volume of sound %d to %d\n", i, g_musicDb);
        }
        if (g_streamOn) {
            if (g_streams[g_streamCount - 1]->SetVolume(g_musicDb) < 0)
                DebugLog("could not set stream volume %d\n", g_musicDb);
        }
    }
}

void SetSfxVolume(float v)
{
    if (g_audioOn) {
        g_sfxVolume = v;
        g_sfxDb = (int)(v * 10000.0f - 10000.0f);
        DebugLog("setting sfx volume %d\n", g_sfxDb);
        int n = g_streamCount;
        if (g_streamOn)
            n--;
        for (int i = 0; i < n; i++) {
            if (g_streams[i]->SetVolume(g_sfxDb) < 0) {
                DebugLog("could not set volume of stream %d to %d\n", i, g_sfxDb);
                return;
            }
        }
    }
}

void SetMasterVolumeAndApply(float v)
{
    g_masterVolume = v;
    SetMasterVolume(v);
}
