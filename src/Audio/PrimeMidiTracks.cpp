// SQZ.EXE 0x45b380: rewind every MIDI reader and decode its first delta time.
#include "compat41.h"
#include "MidiReader.h"

extern const char kMidiRewindFailed[];       // 0x4bf680
int ReadMidiVarLen(MidiReader *t, unsigned *out); // 0x45be20
void DebugLog(const char *format, ...);      // 0x419290

int PrimeMidiTracks()
{
    gMidiNow = 0;
    MidiReader *track = gMidiInfo.tracks;
    for (unsigned i = 0; i < gMidiInfo.trackCount; ++i, ++track) {
        SetFilePointer(gDataFile, track->start, 0, FILE_BEGIN);
        DWORD want = 0x400;
        if (track->length <= 0x400)
            want = track->length;
        DWORD got;
        if (!ReadFile(gDataFile, track->buffer, want, &got, 0) || got != want) {
            DebugLog(kMidiRewindFailed);
            return 1;
        }
        track->left = track->length - got;
        track->buffered = got;
        track->position = SetFilePointer(gDataFile, 0, 0, FILE_CURRENT);
        track->cursor = track->buffer;
        track->flags = 0;
        track->runningStatus = 0;
        track->delta = 0;
        if (!track->buffered && !track->left) {
            track->flags |= 1;
        } else {
            if (ReadMidiVarLen(track, &track->delta)) {
                DebugLog(kMidiRewindFailed);
                return 1;
            }
            SetFilePointer(gDataFile, track->length + track->start, 0, FILE_BEGIN);
        }
    }
    return 0;
}
