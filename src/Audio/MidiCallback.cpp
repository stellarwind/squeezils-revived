// SQZ.EXE 0x45ace0: service MOM_DONE, drain a stopped stream, or queue its next buffer.
#include "compat41.h"
#include <windows.h>
#include <mmsystem.h>

// Same stream-buffer layout as StartMidiStream.cpp.
struct MidiStreamBuf {
    MIDIHDR header;
    int f40;
    int capacity;
    int used;
    int f4c, f50;
};
extern HMIDISTRM gMidiStream;                 // 0x4c86b8
extern MidiStreamBuf gMidiBuffers[32];        // 0x4c7c08
extern int gMidiPlayed;                      // 0x4c86c8
extern int gMidiBuffer;                      // 0x4c86d4
extern int gMidiClock;                       // 0x4c86d8
extern int gMidiPlaying;                     // 0x4bf44c
extern int gMidiQueued;                      // 0x4bf460
extern int gMidiDrainCount;                  // 0x4bf464
extern HANDLE gMidiCallbackEvent;            // 0x4c86cc
extern HWND g_mainWnd;                       // 0x4c73cc
extern const char kMidiCallbackConvertError[]; // 0x4bf558
int FillMidiBuffer(int first, MidiStreamBuf *buffer); // 0x45b4d0
int DebugLogAll(const char *format, ...);      // 0x419290
int StopMidi();                             // 0x45aa70
void FreeMidiTracks();                      // 0x45b2d0

void CALLBACK MidiCallback(HMIDIOUT out, UINT msg, DWORD instance, DWORD p1, DWORD p2)
{
    if (msg != MOM_DONE || gMidiClock == 200)
        return;
    ++gMidiPlayed;
    if (gMidiClock == 300) {
        if (gMidiPlayed >= 32) {
            gMidiClock = 200;
            PostMessage(g_mainWnd, WM_COMMAND, 0, 0);
            SetEvent(gMidiCallbackEvent);
        }
        return;
    }
    if (gMidiClock == 100) {
        if (gMidiPlayed >= 32) {
            gMidiClock = 200;
            SetEvent(gMidiCallbackEvent);
        }
        return;
    }
    if (gMidiPlaying && gMidiPlayed) {
        gMidiBuffers[gMidiBuffer].f40 = 0;
        gMidiBuffers[gMidiBuffer].capacity = 0x400;
        gMidiBuffers[gMidiBuffer].f4c = 0;
        gMidiBuffers[gMidiBuffer].used = 0;
        gMidiBuffers[gMidiBuffer].f50 = 0;
        int result = FillMidiBuffer(0, &gMidiBuffers[gMidiBuffer]);
        if (result) {
            if (result == -0x67) {
                gMidiDrainCount = 31;
                gMidiClock = 300;
            } else {
                DebugLogAll(kMidiCallbackConvertError);
                gMidiQueued = 1;
                StopMidi();
            }
            return;
        }
        gMidiBuffers[gMidiBuffer].header.dwBytesRecorded = gMidiBuffers[gMidiBuffer].used;
        if (midiStreamOut(gMidiStream, &gMidiBuffers[gMidiBuffer].header, sizeof(MIDIHDR))) {
            FreeMidiTracks();
            return;
        }
        gMidiBuffer = (gMidiBuffer + 1) % 32;
        --gMidiPlayed;
    }
}
