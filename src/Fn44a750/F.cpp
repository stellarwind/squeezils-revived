// SQZ.EXE 0x44a750 (cdecl, String argument by value): start a looping ambient stream by sample name unless it is already running.
#include <windows.h>
#include <dsound.h>
#include "String/String.h"

struct AmbSound { int useCount, f4, f8, fc, f10, f14; LPDIRECTSOUNDBUFFER buffer; };
struct AmbientNames {
    String Get(int index);                              // 0x40ec10
    void Insert(const String &name, int at);            // 0x45e280
    int pad, count;                                     // count at +4 (0x4c77b4)
};
struct StreamBuffers { void Insert(LPDIRECTSOUNDBUFFER *item, int at); };   // 0x4680b0
extern AmbientNames gAmbientNames;                      // 0x4c77b0
extern StreamBuffers gStreamBuffers;                    // 0x4c77d0
extern int g_audioOn;                                   // 0x4bd95c
extern int g_streamOn;                                  // 0x4bd960
extern int g_ambientDb;                                 // 0x4c77f0
extern const char kAmbBadVolume[], kAmbBadPlay[], kAmbBadStop[], kAmbMissing[];   // 0x4bdfe4 0x4bdfb0 0x4be050 0x4be068
int StringEquals(const String &a, const String &b);     // 0x438720 (cdecl)
AmbSound *FindSound(const char *name);                  // 0x449c30 (cdecl)
void DebugLog(const char *format, ...);                 // 0x419290

void StartAmbient(String name)
{
    if (!g_audioOn)
        return;
    for (int i = 0; i < gAmbientNames.count; i++)
        if (StringEquals(gAmbientNames.Get(i), name))
            return;
    AmbSound *sound = FindSound(name.c_str());
    if (sound) {
        sound->f4 = 0;
        sound->f8 = 0;
        sound->fc = 0;
        sound->f10 = 0xbf800000;
        LPDIRECTSOUNDBUFFER *slot = &sound->buffer;
        if ((*slot)->SetVolume(g_ambientDb) < 0) {
            DebugLog(kAmbBadVolume, g_ambientDb);
            return;
        }
        if ((*slot)->Play(0, 0, DSBPLAY_LOOPING) < 0) {
            DebugLog(kAmbBadPlay);
            return;
        }
        if ((*slot)->Stop() < 0) {
            DebugLog(kAmbBadStop);
            return;
        }
        gStreamBuffers.Insert(slot, -1);
        gAmbientNames.Insert(name, -1);
        g_streamOn = 1;
    } else
        DebugLog(kAmbMissing, name.c_str());
}
