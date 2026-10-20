// 0x45aa70: stop and release the MIDI stream, then start a queued track.
#include <windows.h>
#include <mmsystem.h>

extern HMIDISTRM gMidiStream;       // 0x4c86b8
extern HANDLE gMidiCallbackEvent;  // 0x4c86cc
extern int gMidiClock;             // 0x4c86d8
extern int gMidiPlaying;           // 0x4bf44c
extern int gMidiLoaded;            // 0x4bf458
extern int gMidiQueued;            // 0x4bf460
extern char gQueuedMidiFile[];     // 0x4c86e0
extern const char kMidiStopError[], kMidiResetError[], kMidiWaitError[], kMidiCloseError[];
void GameLog(const char *format, ...); // 0x419290
void ReleaseMidiFile();            // 0x45b2d0
void ReleaseMidiBuffers();         // 0x45c250
int StartMidi(const char *file);   // 0x45aa00

int StopMidi()
{
    int playing = gMidiPlaying;
    int state = gMidiClock;
    if (playing || state != 200) {
        gMidiPlaying = 0;
        if (state != 200 && state != 300)
            gMidiClock = 100;
        if (gMidiStream) {
            if (midiStreamStop(gMidiStream))
                GameLog(kMidiStopError);
            if (midiOutReset((HMIDIOUT)gMidiStream))
                GameLog(kMidiResetError);
        }
        DWORD waitResult = WaitForSingleObject(gMidiCallbackEvent, 2000);
        state = gMidiClock;
        if (waitResult == WAIT_TIMEOUT) {
            GameLog(kMidiWaitError);
            state = 200;
        }
    }
    gMidiClock = state;
    if (state == 200) {
        gMidiClock = 0;
        ReleaseMidiFile();
        ReleaseMidiBuffers();
        if (gMidiStream) {
            if (midiStreamClose(gMidiStream))
                GameLog(kMidiCloseError);
            gMidiStream = 0;
        }
    }
    gMidiLoaded = 0;
    if (gMidiQueued) {
        StartMidi(gQueuedMidiFile);
        gMidiQueued = 0;
    }
    return 1;
}
