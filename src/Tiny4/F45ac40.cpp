// 0x45ac40: set the MIDI master volume and silence-on flag, then send the per-channel volume controller.
#include <windows.h>
#include <mmsystem.h>
int DebugLogAll(const char *fmt, ...);     // 0x419290
extern int g_musicOff;                   // 0x4bf45c
extern HMIDIOUT g_midi;                  // 0x4c86b8
int SetMusicVolume(float vol)
{
    unsigned ch;
    int v, v7;
    g_musicOff = 1;
    if (vol != 0.0f)
        g_musicOff = 0;
    v = (int)(vol * 65535.0f);
    midiOutSetVolume((HMIDIOUT)g_midi, (v << 16) + v);
    v7 = (int)(vol * 127.0f) << 16;
    for (ch = 0; ch < 16; ch++) {
        if (midiOutShortMsg(g_midi, ch | v7 | 0x7b0)) {
            DebugLogAll("MIDI volume failed on channel %d\n", ch);
            return 0;
        }
    }
    return 1;
}
