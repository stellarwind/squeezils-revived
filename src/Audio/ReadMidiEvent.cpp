// SQZ.EXE 0x45ba90 (cdecl): read the next event of a MIDI track (running status, channel messages, sysex and
// meta events) and the delta time to the event after it. 0 ok, 1 at the end of the track or on a read error.
struct MidiEvent {
    unsigned time;                          // +0
    unsigned char status, d1, d2, d3;       // +4
    unsigned length;                        // +8
    unsigned char *data;                    // +0xc
};
struct MidiTrack {
    unsigned flags;                         // +0 (bit 0: track ended)
    int pad4;
    unsigned left;                          // +8 bytes left in the file
    char padc[0x14 - 0xc];
    unsigned delta;                         // +0x14
    unsigned char runningStatus;            // +0x18
    char pad19[0x24 - 0x19];
    unsigned buffered;                      // +0x24 bytes left in the read buffer
};
extern int gMidiAllocs;                     // 0x4bf448
int ReadMidiByte(MidiTrack *t, unsigned char *out);     // 0x45beb0
int ReadMidiVarLen(MidiTrack *t, unsigned *out);        // 0x45be20
void *AllocBlock(unsigned size);                        // 0x4904c0

int ReadMidiEvent(MidiTrack *t, MidiEvent *ev)
{
    ev->time = 0;
    *(unsigned *)&ev->status = 0;
    ev->length = 0;
    ev->data = 0;
    if ((t->flags & 1) || (!t->left && !t->buffered))
        return 1;
    unsigned char b;
    if (ReadMidiByte(t, &b))
        return 1;
    if (!(b & 0x80)) {
        unsigned char status = t->runningStatus;
        if (!status)
            return 1;
        ev->status = status;
        ev->d1 = b;
        b = status & 0xf0;
        ev->length = 2;
        if (b != 0xc0 && b != 0xd0) {
            if (!t->left && !t->buffered) {
                t->flags |= 1;
                return 1;
            }
            if (ReadMidiByte(t, &ev->d2))
                return 1;
            ev->length++;
        }
    } else if ((b & 0xf0) != 0xf0) {
        unsigned char type = b & 0xf0;
        ev->status = b;
        b = type;
        t->runningStatus = ev->status;
        unsigned n;
        if (type == 0xc0 || (n = 2, type == 0xd0))
            n = 1;
        ev->length = n + 1;
        if (t->buffered + t->left < n) {
            t->flags |= 1;
            return 1;
        }
        if (ReadMidiByte(t, &ev->d1))
            return 1;
        if (n == 2 && ReadMidiByte(t, &ev->d2))
            return 1;
    } else if (b == 0xf0 || b == 0xf7) {
        ev->status = b;
        if (ReadMidiVarLen(t, &ev->length))
            return 1;
        if (t->buffered + t->left < ev->length) {
            t->flags |= 1;
            return 1;
        }
        if (!(ev->data = (unsigned char *)AllocBlock(ev->length)))
            return 1;
        for (unsigned i = 0; i < ev->length; i++)
            if (ReadMidiByte(t, ev->data + i))
                return 1;
        gMidiAllocs++;
    } else if (b == 0xff) {
        ev->status = b;
        if (!t->left && !t->buffered) {
            t->flags |= 1;
            return 1;
        }
        if (ReadMidiByte(t, &ev->d1))
            return 1;
        if (ReadMidiVarLen(t, &ev->length))
            return 1;
        if (ev->length) {
            if (t->buffered + t->left < ev->length) {
                t->flags |= 1;
                return 1;
            }
            if (!(ev->data = (unsigned char *)AllocBlock(ev->length)))
                return 1;
            for (unsigned i = 0; i < ev->length; i++)
                if (ReadMidiByte(t, ev->data + i))
                    return 1;
            gMidiAllocs++;
        }
        if (ev->d1 == 0x2f)
            t->flags |= 1;
    } else {
        return 1;
    }
    ev->time = t->delta;
    if (!(t->flags & 1)) {
        unsigned delta;
        if (ReadMidiVarLen(t, &delta))
            return 1;
        t->delta += delta;
    }
    return 0;
}
