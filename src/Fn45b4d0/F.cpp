// SQZ.EXE 0x45b4d0 (cdecl, 2 args): fill one MIDI stream buffer with converted events from the track with the earliest next event.
// Returns 0 ok, 1 error, -101/-102 (-0x65/-0x66) failure; -0x68 (buffer full) is held back in the flags.
struct MidiEvent {
    unsigned time;                          // +0
    unsigned char status, d1, d2, d3;       // +4
    int pad8;
    unsigned char *data;                    // +0xc
};
struct MidiTrack {
    unsigned flags;                         // +0
    char pad4[0x14 - 4];
    unsigned delta;                         // +0x14
    char pad18[0x28 - 0x18];
};
struct MidiBuf {
    char pad[0x48];
    unsigned used;                          // +0x48
};
extern MidiEvent gMidiEvent;                // 0x4c86a8
extern unsigned gMidiFlags;                 // 0x4c86d0
extern MidiTrack *gBestTrack;               // 0x4c86c0
extern MidiTrack *gScanTrack;               // 0x4c86c4
extern unsigned gMinDelta;                  // 0x4c7c00
extern MidiTrack *gMidiTracks;              // 0x4c86a0
extern unsigned gMidiTrackCount;            // 0x4c869c
extern int gMidiAllocs;                     // 0x4bf448
extern const char kNoTracks[], kReadFail[], kConvertFail[];   // 0x4bf6c8 0x4bf69c 0x4bf704
void DebugLog(const char *format, ...);     // 0x419290
void FreeBlock(void *p);                    // 0x490540
int PrimeMidiTracks();                      // 0x45b380
int ReadMidiEvent(MidiTrack *t, MidiEvent *ev);   // 0x45ba90 (cdecl)
int ConvertMidiEvent(MidiEvent *ev, MidiBuf *out);   // 0x45b880 (cdecl)

#define DROP_EVENT() \
    if (gMidiAllocs) { \
        FreeBlock(gMidiEvent.data); \
        gMidiAllocs--; \
    }

int FillMidiBuffer(unsigned flags, MidiBuf *out)
{
    out->used = 0;
    if (flags & 1) {
        gMidiEvent.time = 0;
        gMidiFlags = 0;
        gBestTrack = 0;
        *(unsigned *)&gMidiEvent.status = 0;
        gScanTrack = 0;
        gMidiEvent.pad8 = 0;
        gMidiEvent.data = 0;
    }
    if (gMidiFlags & 1) {
        PrimeMidiTracks();
        gMidiFlags = 0;
    } else {
        if (gMidiFlags & 2)
            return -102;
        if (gMidiFlags & 4) {
            gMidiFlags ^= 4;
            if (gMidiEvent.status == 0xff && gMidiEvent.d1 == 0x2f) {
                DROP_EVENT()
            } else {
                int r = ConvertMidiEvent(&gMidiEvent, out);
                if (r) {
                    if (r == -104) {
                        gMidiFlags |= 4;
                        return 0;
                    }
                    if (r != -105) {
                        DebugLog(kConvertFail);
                        DROP_EVENT()
                        return 1;
                    }
                }
            }
        }
    }
    for (;;) {
        gBestTrack = 0;
        gMinDelta = 0xffffffff;
        if (!gMidiTracks) {
            DebugLog(kNoTracks);
            return -101;
        }
        gScanTrack = gMidiTracks;
        for (unsigned n = gMidiTrackCount; n; n--) {
            if (!(gScanTrack->flags & 1) && gMinDelta > gScanTrack->delta) {
                gMinDelta = gScanTrack->delta;
                gBestTrack = gScanTrack;
            }
            gScanTrack++;
        }
        if (!gBestTrack) {
            gMidiFlags |= 1;
            return 0;
        }
        if (ReadMidiEvent(gBestTrack, &gMidiEvent)) {
            gMidiFlags |= 2;
            DebugLog(kReadFail);
            return -101;
        }
        if (gMidiEvent.status == 0xff && gMidiEvent.d1 == 0x2f) {
            DROP_EVENT()
            continue;
        }
        int r = ConvertMidiEvent(&gMidiEvent, out);
        if (!r)
            continue;
        if (r == -104) {
            gMidiFlags |= 4;
            return 0;
        }
        if (r == -105)
            continue;
        DebugLog(kConvertFail);
        DROP_EVENT()
        return 1;
    }
}
