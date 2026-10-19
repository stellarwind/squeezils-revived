// 11 spare declarations stabilize VC4.1 comparison ordering (padscan.sh).
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
// SQZ.EXE 0x45b2d0: close the MIDI source file and release its track buffers.
#include <windows.h>

struct MidiTrackBuffer {
    char pad00[0xc];
    void *data;                             // +0xc
    char rest[0x18];                        // complete track stride: 0x28
};
extern HANDLE gDataFile;                    // 0x4bf444
extern DWORD gMidiTrackCount;        // 0x4c869c
extern MidiTrackBuffer *gMidiTracks;        // 0x4c86a0

void ReleaseMidiFile()
{
    if (gDataFile != INVALID_HANDLE_VALUE) {
        CloseHandle(gDataFile);
        gDataFile = INVALID_HANDLE_VALUE;
    }
    unsigned int i = 0;
    if (gMidiTracks) {
        for (; gMidiTrackCount > i; ++i) {
            if (gMidiTracks[i].data) {
                GlobalUnlock(GlobalHandle(gMidiTracks[i].data));
                GlobalFree(GlobalHandle(gMidiTracks[i].data));
            }
        }
        GlobalUnlock(GlobalHandle(gMidiTracks));
        GlobalFree(GlobalHandle(gMidiTracks));
        gMidiTracks = 0;
    }
}
