// Joystick capture (SQZ.EXE 0x4479d0): one shared `r` with a single `return r` after the if/else matches (separate returns do not).
#include <windows.h>
#include <mmsystem.h>
struct AppWnd { char pad[0x18]; HWND hwnd; };
extern AppWnd *gAppWnd;
struct Joystick {
    char pad[0x28];
    UINT threshold;
    int present;
    int Capture(int on);
};
int Joystick::Capture(int on)
{
    int r;
    if (!present)
        return 0;
    if (on) {
        r = joySetCapture(gAppWnd->hwnd, 0, 0, TRUE);
        joySetThreshold(0, threshold);
    } else {
        r = joyReleaseCapture(0);
    }
    return r;
}

// 0x45abe0: restart the MIDI stream
int DebugLogAll(const char *fmt, ...);                               // 0x419290
extern volatile HMIDISTRM gMidiStream;                                      // 0x4c86b8
extern volatile int gMidiStopped;                                           // 0x4bf45c
extern int gMidiClock;                                             // 0x4c86d8
extern int gMidiPlaying;                                           // 0x4bf44c
int ResumeMidi()
{
    if (!gMidiStream || gMidiStopped)
        return 0;
    gMidiClock = 0;
    if (midiStreamRestart(gMidiStream)) {
        DebugLogAll("Error resuming stream\n");
        return 0;
    }
    gMidiPlaying = 1;
    return 1;
}
