// SQZ.EXE 0x45be20: read a 7-bit-per-byte variable-length integer (high bit = continue) from a stream; returns 1 on error (read failure latches the error flag).
struct Stream {
    unsigned flags;             // bit 0 = error
    int pad4;
    int src8;                   // +8
    char pad0c[0x24 - 0xc];
    int src24;                  // +0x24
};
int ReadByte(Stream *s, unsigned char *b);      // 0x45beb0
int ReadVarInt(Stream *s, unsigned *out)
{
    unsigned v = 0;
    unsigned char b;
    if (s->flags & 1)
        return 1;
    do {
        if (!s->src8 && !s->src24) {
            s->flags |= 1;
            return 1;
        }
        if (ReadByte(s, &b)) {
            s->flags |= 1;
            return 1;
        }
        v = (v << 7) | (b & 0x7f);
    } while (b & 0x80);
    *out = v;
    return 0;
}
