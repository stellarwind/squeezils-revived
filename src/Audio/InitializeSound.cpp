// SQZ.EXE 0x4495e0: load volume defaults and initialize the DirectSound primary buffer.
#include "compat41.h"
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <mmreg.h>
#include <msacm.h>
#include <string.h>
#include "IniReader/IniReader.h"

extern LPDIRECTSOUND g_directSound;              // 0x4bd95c
extern LPDIRECTSOUNDBUFFER g_primarySound;       // 0x4c77ac
extern WAVEFORMATEX *g_sampleFormat;            // 0x4c77a8
extern DSBUFFERDESC g_primarySoundDesc;         // 0x4c77f8
extern HWND g_mainWnd;                         // 0x4c73cc
struct MidiApp;
extern MidiApp *g_app;                         // 0x4c75dc
extern float g_musicVolume, g_sfxVolume, g_masterVolume; // 0x4c77cc, 0x4c77c8, 0x4c77a0
extern int g_musicDb, g_sfxDb;                  // 0x4c77f0, 0x4c77a4
extern const char kSoundDefaults[], kInitialEffect[], kInitialAmbient[], kInitialMidi[];
    // 0x4b9e60, 0x4bde58, 0x4bde3c, 0x4bde24
extern const char kSoundInitFailed[], kSoundCooperativeFailed[], kPrimaryCreateFailed[],
    kFormatSizeFailed[], kFormatAllocFailed[], kPrimaryFormatFailed[], kPrimaryPlayFailed[];
    // 0x4bdd68, 0x4bdd3c, 0x4bdd1c, 0x4bdcec, 0x4bdcbc, 0x4bdc90, 0x4bdc70
int FatalErrorApp(MidiApp *app, const char *fmt, ...); // 0x475510
void DebugLog(const char *format, ...);          // 0x419290

void InitializeSound()
{
    IniReader ini;
    ini.SetSection(kSoundDefaults);
    float clamped;
    g_musicVolume = ini.GetFloat(kInitialEffect, 0.5f);
    if (g_musicVolume < 0.0f) clamped = 0.0f;
    else clamped = 1.0f <= g_musicVolume ? 1.0f : g_musicVolume;
    g_musicVolume = clamped;
    g_sfxVolume = ini.GetFloat(kInitialAmbient, 0.15f);
    if (g_sfxVolume < 0.0f) clamped = 0.0f;
    else clamped = 1.0f <= g_sfxVolume ? 1.0f : g_sfxVolume;
    g_sfxVolume = clamped;
    g_masterVolume = ini.GetFloat(kInitialMidi, 0.25f);
    if (g_masterVolume < 0.0f) clamped = 0.0f;
    else clamped = 1.0f <= g_masterVolume ? 1.0f : g_masterVolume;
    g_masterVolume = clamped;

    while (!g_directSound) {
        HRESULT hr = DirectSoundCreate(0, &g_directSound, 0);
        if ((!g_directSound || hr < 0) && FatalErrorApp(g_app, kSoundInitFailed) == 2)
            return;
    }
    if (g_directSound->SetCooperativeLevel(g_mainWnd, DSSCL_PRIORITY) < 0) {
        DebugLog(kSoundCooperativeFailed);
        g_directSound->Release();
        return;
    }
    memset(&g_primarySoundDesc, 0, sizeof(g_primarySoundDesc));
    g_primarySoundDesc.dwBufferBytes = 0;
    g_primarySoundDesc.lpwfxFormat = 0;
    g_primarySoundDesc.dwSize = 0x14;
    g_primarySoundDesc.dwFlags = 0x81;
    if (g_directSound->CreateSoundBuffer(&g_primarySoundDesc, &g_primarySound, 0) < 0) {
        DebugLog(kPrimaryCreateFailed);
        g_directSound->Release();
        return;
    }
    DWORD formatSize;
    if (acmMetrics(0, ACM_METRIC_MAX_SIZE_FORMAT, &formatSize)) {
        DebugLog(kFormatSizeFailed);
        return;
    }
    g_sampleFormat = (WAVEFORMATEX *)GlobalLock(GlobalAlloc(GMEM_ZEROINIT, formatSize));
    if (!g_sampleFormat) {
        DebugLog(kFormatAllocFailed);
        return;
    }
    g_sampleFormat->wFormatTag = WAVE_FORMAT_PCM;
    g_sampleFormat->nChannels = 1;
    g_sampleFormat->nSamplesPerSec = 22050;
    g_sampleFormat->nAvgBytesPerSec = 44100;
    g_sampleFormat->nBlockAlign = 2;
    g_sampleFormat->wBitsPerSample = 16;
    g_sampleFormat->cbSize = 0;
    if (g_primarySound->SetFormat(g_sampleFormat) < 0) {
        DebugLog(kPrimaryFormatFailed);
        GlobalUnlock(GlobalHandle(g_sampleFormat));
        GlobalFree(GlobalHandle(g_sampleFormat));
        g_directSound->Release();
        return;
    }
    GlobalUnlock(GlobalHandle(g_sampleFormat));
    GlobalFree(GlobalHandle(g_sampleFormat));
    g_directSound->SetCooperativeLevel(g_mainWnd, DSSCL_NORMAL);
    double attenuation = (1.0 - g_musicVolume) * 100.0;
    g_musicDb = -(int)(attenuation * attenuation);
    attenuation = (1.0 - g_sfxVolume) * 100.0;
    g_sfxDb = -(int)(attenuation * attenuation);
    if (g_primarySound->Play(0, 0, DSBPLAY_LOOPING) < 0) {
        DebugLog(kPrimaryPlayFailed);
        GlobalUnlock(GlobalHandle(g_sampleFormat));
        GlobalFree(GlobalHandle(g_sampleFormat));
        g_directSound->Release();
    }
}
