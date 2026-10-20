// SQZ.EXE 0x449aa0 (cdecl): load a .wav sample, build its DirectSound buffer and register it in the current sound scope; 0 on failure.
struct String {
    String(const char *s);                  // 0x438380
    ~String();                              // 0x4383d0
    char *rep;
};
struct WaveSound {
    int useCount, id, f8, fc, f10, f14;
    char rest[0x68 - 0x18];
    WaveSound();                            // 0x448d90
    ~WaveSound();                           // 0x448db0 (empty)
    int Load(int flag);                     // 0x4490f0
    int CreateSecondary();                  // 0x449200
    int Free();                             // 0x4493a0
};
struct SoundScope {
    virtual void Add(const String &key, WaveSound *&value, int index);
};
struct Renderer { int FatalError(const char *fmt, ...); };   // 0x475510
extern Renderer *g_app;                     // 0x4c75dc
extern int g_audioOn;                       // 0x4bd95c
int DebugLogAll(const char *format, ...);     // 0x419290
SoundScope *CurrentSoundScope();            // 0x44aad0
extern const char kLoadingSound[], kSoundRetry[], kNoSecondaryBuffer[], kNoFill[], kSoundOk[];   // 0x4bdecc 0x4bdeb4 0x4bde8c 0x4bde70 0x4b9dbc

int LoadSound(const char *name, int flag)
{
    if (!g_audioOn)
        return 0;
    WaveSound *w = new WaveSound;
    DebugLogAll(kLoadingSound, name, flag);
    int ok = 0;
    while (!ok) {
        ok = w->Load(flag);
        if (!ok && g_app->FatalError(kSoundRetry, flag) == 2)
            return 0;
    }
    if (!w->CreateSecondary()) {
        DebugLogAll(kNoSecondaryBuffer);
        if (w)
            delete w;
        return 0;
    }
    if (!w->Free()) {
        DebugLogAll(kNoFill);
        if (w)
            delete w;
        return 0;
    }
    DebugLogAll(kSoundOk);
    CurrentSoundScope()->Add(String(name), w, -1);
    w->useCount++;
    return 1;
}
