// SQZ.EXE 0x45b880: convert one parsed MIDI event into a 12-byte stream record; 0 ok, -104 buffer full, -105 unsupported event.
struct MidiEvent {
    unsigned time;                          // +0
    unsigned char status, d1, d2, d3;       // +4
    int pad8;
    unsigned char *data;                    // +0xc
};
struct MidiBuf {
    unsigned char *base;                    // +0
    char pad4[0x40 - 4];
    unsigned offset;                        // +0x40
    unsigned size;                          // +0x44
    unsigned used;                          // +0x48
    unsigned lastTime;                      // +0x4c
    int overflow;                           // +0x50
};
extern unsigned gMidiNow;                   // 0x4c86dc
extern unsigned gMidiLimit;                 // 0x4c86bc
extern unsigned gMidiTempo;                 // 0x4c8688
extern unsigned gMidiDivision;              // 0x4c8694
extern int gMidiAllocs;                     // 0x4bf448
extern const char kMidiSysex[];             // 0x4bf74c
void DebugLog(const char *format, ...);     // 0x419290
void FreeBlock(void *p);                    // 0x490540

int ConvertMidiEvent(MidiEvent *ev, MidiBuf *out)
{
    unsigned *dst = (unsigned *)(out->offset + (unsigned)out->base + out->used);
    if (out->used == 0)
        out->lastTime = gMidiNow;
    if (gMidiNow - out->lastTime > gMidiLimit) {
        if (out->overflow) {
            out->overflow = 0;
            return -104;
        }
        out->overflow = 1;
    }
    unsigned delta = ev->time - gMidiNow;
    gMidiNow = ev->time;
    if (ev->status < 0xf0) {
        if (out->size - out->used < 0xc)
            return -104;
        dst[0] = delta;
        dst[1] = 0;
        dst[2] = ev->status | (ev->d1 << 8) | (ev->d2 << 16);
        out->used += 0xc;
        return 0;
    }
    if (ev->status == 0xf0 || ev->status == 0xf7) {
        DebugLog(kMidiSysex);
        if (gMidiAllocs) {
            FreeBlock(ev->data);
            gMidiAllocs--;
        }
        return 0;
    }
    if (ev->d1 != 0x51) {
        if (gMidiAllocs) {
            FreeBlock(ev->data);
            gMidiAllocs--;
        }
        return -105;
    }
    if (out->size - out->used < 0xc) {
        if (gMidiAllocs) {
            FreeBlock(ev->data);
            gMidiAllocs--;
        }
        return -104;
    }
    dst[0] = delta;
    dst[1] = 0;
    unsigned char *p = ev->data;
    unsigned v = (p[0] << 16) | (p[1] << 8) | p[2];
    dst[2] = v;
    gMidiTempo = v;
    dst[2] = dst[2] * 100 / 100;
    dst[2] |= 0x1000000;
    gMidiLimit = gMidiDivision * 60000 / gMidiTempo;
    if (gMidiAllocs) {
        FreeBlock(ev->data);
        gMidiAllocs--;
    }
    out->used += 0xc;
    return 0;
}
