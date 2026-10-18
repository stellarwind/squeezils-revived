// SQZ.EXE 0x47a1b0: pop the previous-frame stack of a file reader (error text names the file at +0x208).
void ReportError(const char *fmt, const char *name);   // 0x47a7b0
struct FrameReader {
    int pad0, ok;               // ok = +4
    char pad8[0x208 - 8];
    char name[0x110];           // +0x208
    int cur;                    // +0x318
    int depth;                  // +0x31c
    int stack[1];               // +0x320
    void PopFrame(int unused);
};
void FrameReader::PopFrame(int)
{
    if (depth == 0) {
        ReportError("Underflow of previous frame stack while reading %s", name);
        ok = 0;
        return;
    }
    depth--;
    cur = stack[depth];
}
