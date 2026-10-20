// SQZ.EXE 0x45bf40 (cdecl): open the MIDI stream, allocate the 32 stream buffers, start the converter on the
// current file, then convert, prepare and queue buffers until the song ends or all are queued.
// 0 ok, 1 on a stream error (stream closed and converter freed), -1 when a buffer cannot be allocated.
#include <windows.h>
#include <mmsystem.h>

struct MidiStreamBuf {
    MIDIHDR header;           // 0x00
    int f40;                  // 0x40
    int capacity;             // 0x44
    int used;                 // 0x48
    int f4c, f50;             // 0x4c
};
extern HMIDISTRM gMidiStream;                 // 0x4c86b8
extern UINT gMidiDevice;                      // 0x4bf454
extern MidiStreamBuf gMidiBuffers[32];        // 0x4c7c08
extern char gMidiFile[];                      // 0x4c86e0
extern DWORD gMidiTimeDivision;               // 0x4c8694
extern int gMidiPlayed;                       // 0x4c86c8
extern int gMidiBuffer;                       // 0x4c86d4
extern int gMidiHeadersPrepared;              // 0x4bf450
extern const char kOpenError[], kAllocError[], kInitError[], kCloseInitError[], kPropertyError[],
    kClosePropertyError[], kConvertError[], kCloseConvertError[], kPrepareError[], kClosePrepareError[];
    // 0x4bf934 0x4bf90c 0x4bf8ec 0x4bf8b4 0x4bf894 0x4bf85c 0x4bf838 0x4bf7fc 0x4bf7d8 0x4bf798
void DebugLog(const char *format, ...);       // 0x419290
void CALLBACK MidiCallback(HMIDIOUT out, UINT msg, DWORD instance, DWORD p1, DWORD p2);   // 0x45ace0
int InitMidiConverter(const char *file);      // 0x45ae90
void FreeMidiTracks();                        // 0x45b2d0
void FreeMidiConverter();                     // 0x45c250
int FillMidiBuffer(int first, MidiStreamBuf *buffer);   // 0x45b4d0

int StartMidiStream()
{
    int done = 0;
    if (!gMidiStream && midiStreamOpen(&gMidiStream, &gMidiDevice, 1, (DWORD)MidiCallback, 0, CALLBACK_FUNCTION)) {
        DebugLog(kOpenError);
        return 1;
    }
    int i = 0;
    MidiStreamBuf *buffer = gMidiBuffers;
    do {
        buffer->header.dwBufferLength = 0x400;
        buffer->header.lpData = (LPSTR)GlobalLock(GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 0x400));
        if (!buffer->header.lpData) {
            DebugLog(kAllocError, i);
            return -1;
        }
        buffer++;
        i++;
    } while (buffer < gMidiBuffers + 32);
    if (InitMidiConverter(gMidiFile)) {
        DebugLog(kInitError);
        if (midiStreamClose(gMidiStream))
            DebugLog(kCloseInitError);
        gMidiStream = 0;
        FreeMidiConverter();
        return 1;
    }
    MIDIPROPTIMEDIV division;
    division.cbStruct = 8;
    division.dwTimeDiv = gMidiTimeDivision;
    if (midiStreamProperty(gMidiStream, (LPBYTE)&division, MIDIPROP_SET | MIDIPROP_TIMEDIV)) {
        DebugLog(kPropertyError);
        if (midiStreamClose(gMidiStream))
            DebugLog(kClosePropertyError);
        gMidiStream = 0;
        FreeMidiTracks();
        FreeMidiConverter();
        return 1;
    }
    gMidiPlayed = 0;
    int first = 1;
    gMidiBuffer = 0;
    do {
        gMidiBuffers[gMidiBuffer].f40 = 0;
        gMidiBuffers[gMidiBuffer].capacity = 0x400;
        gMidiBuffers[gMidiBuffer].f4c = 0;
        gMidiBuffers[gMidiBuffer].f50 = 0;
        int result = FillMidiBuffer(first, &gMidiBuffers[gMidiBuffer]);
        if (result) {
            if (result != -0x67) {
                DebugLog(kConvertError);
                if (midiStreamClose(gMidiStream))
                    DebugLog(kCloseConvertError);
                gMidiStream = 0;
                FreeMidiTracks();
                FreeMidiConverter();
                return 1;
            }
            done = 1;
        }
        gMidiBuffers[gMidiBuffer].header.dwBytesRecorded = gMidiBuffers[gMidiBuffer].used;
        if (!gMidiHeadersPrepared
            && midiOutPrepareHeader((HMIDIOUT)gMidiStream, &gMidiBuffers[gMidiBuffer].header, sizeof(MIDIHDR))) {
            DebugLog(kPrepareError);
            if (midiStreamClose(gMidiStream))
                DebugLog(kClosePrepareError);
            gMidiStream = 0;
            FreeMidiTracks();
            FreeMidiConverter();
            return 1;
        }
        if (midiStreamOut(gMidiStream, &gMidiBuffers[gMidiBuffer].header, sizeof(MIDIHDR)))
            break;
        first = 0;
        if (done)
            break;
    } while (++gMidiBuffer < 32);
    gMidiHeadersPrepared = 1;
    gMidiBuffer = 0;
    return 0;
}
