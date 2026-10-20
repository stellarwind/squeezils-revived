// SQZ.EXE 0x45ae90 (cdecl): open a standard MIDI file, read the MThd header and set up one buffered reader per
// MTrk chunk (first 1 KB read, first delta time decoded). 0 ok, 1 on any error (readers freed).
#include "MidiReader.h"
struct MidiApp;
extern MidiApp *g_app;                       // 0x4c75dc
int FatalErrorApp(MidiApp *app, const char *fmt, ...);   // 0x475510
void DebugLog(const char *format, ...);      // 0x419290
void FreeMidiTracks();                       // 0x45b2d0
int ReadExact(void *out, unsigned size);     // 0x45bef0
int ReadMidiVarLen(MidiReader *t, unsigned *out);   // 0x45be20
extern const char kCouldNotReadFile[];       // 0x4b9fec
extern const char kMidiOpenFailed[], kMidiSizeFailed[], kMidiBadHeader[], kMidiNoTracks[],
    kMidiNoBuffer[], kMidiBadTrack[], kMidiReadFailed[];
    // 0x4bf660 0x4bf644 0x4bf614 0x4bf5e4 0x4bf5bc 0x4bf5a0 0x4bf57c

#define SWAP32(x) ((((x) & 0xff00 | (x) << 16) << 8) | (((x) & 0xff0000 | (x) >> 16) >> 8))
#define SWAP16(x) ((((x) & 0xff) << 8) | ((x) >> 8))

int InitMidiConverter(const char *file)
{
    gMidiInfo.size = 0;
    gMidiNow = 0;
    gMidiInfo.timeDivision = 0;
    gMidiInfo.format = 0;
    gMidiInfo.trackCount = 0;
    gMidiInfo.tracks = 0;
    gDataFile = INVALID_HANDLE_VALUE;
    gMidiInfo.size = 0;
    gMidiInfo.tracks = 0;
    while (gDataFile == INVALID_HANDLE_VALUE) {
        gDataFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (gDataFile == INVALID_HANDLE_VALUE && FatalErrorApp(g_app, kCouldNotReadFile, file) == 2) {
            DebugLog(kMidiOpenFailed, file);
            FreeMidiTracks();
            return 1;
        }
    }
    gMidiInfo.size = GetFileSize(gDataFile, 0);
    if (gMidiInfo.size == 0xffffffff) {
        DebugLog(kMidiSizeFailed);
        FreeMidiTracks();
        return 1;
    }
    DWORD tag, length;
    WORD header[4];
    if (ReadExact(&tag, 4) || tag != 0x6468544d || ReadExact(&length, 4)
        || (length = SWAP32(length)) < 6 || ReadExact(header, length)) {
        DebugLog(kMidiBadHeader);
        FreeMidiTracks();
        return 1;
    }
    gMidiInfo.format = SWAP16(header[0]);
    gMidiInfo.trackCount = SWAP16(header[1]);
    gMidiInfo.timeDivision = SWAP16(header[2]);
    gMidiInfo.tracks = (MidiReader *)GlobalLock(GlobalAlloc(GMEM_ZEROINIT, gMidiInfo.trackCount * sizeof(MidiReader)));
    if (!gMidiInfo.tracks) {
        DebugLog(kMidiNoTracks);
        FreeMidiTracks();
        return 1;
    }
    MidiReader *t = gMidiInfo.tracks;
    for (unsigned i = 0; i < gMidiInfo.trackCount; i++, t++) {
        t->buffer = (unsigned char *)GlobalLock(GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 0x400));
        if (!t->buffer) {
            DebugLog(kMidiNoBuffer);
            FreeMidiTracks();
            return 1;
        }
        if (ReadExact(&tag, 4) || tag != 0x6b72544d || ReadExact(&length, 4)) {
            DebugLog(kMidiBadTrack);
            FreeMidiTracks();
            return 1;
        }
        length = SWAP32(length);
        t->length = length;
        t->start = SetFilePointer(gDataFile, 0, 0, FILE_CURRENT);
        DWORD want = 0x400;
        if (t->length <= 0x400)
            want = t->length;
        DWORD got;
        if (!ReadFile(gDataFile, t->buffer, want, &got, 0) || want != got) {
            DebugLog(kMidiReadFailed);
            FreeMidiTracks();
            return 1;
        }
        t->left = t->length - got;
        t->buffered = got;
        t->position = SetFilePointer(gDataFile, 0, 0, FILE_CURRENT);
        t->cursor = t->buffer;
        t->flags = 0;
        t->runningStatus = 0;
        t->delta = 0;
        if (!t->buffered && !t->left) {
            t->flags |= 1;
        } else {
            if (ReadMidiVarLen(t, &t->delta)) {
                DebugLog(kMidiReadFailed);
                FreeMidiTracks();
                return 1;
            }
            SetFilePointer(gDataFile, t->length + t->start, 0, FILE_BEGIN);
        }
    }
    return 0;
}
