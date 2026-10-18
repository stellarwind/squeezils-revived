// SQZ.EXE 0x47a120: push the current frame on the previous-frame stack of a file reader, then switch to the new one.
void ReportError(const char *fmt, const char *name);   // 0x47a7b0
struct Frame { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(Frame *cur, Frame **slot); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(Frame *self, int b); };
struct Handle { int **Get(); };               // 0x47e780
struct FrameReader2 {
    int pad0, ok;
    char pad8[0x208 - 8];
    char name[0x100];                           // +0x208
    Handle *newest;                             // +0x308
    Frame *owner;                               // +0x30c
    char pad310[0x318 - 0x310];
    Frame *cur;                                 // +0x318
    int depth;                                  // +0x31c
    Frame *stack[20];                           // +0x320
    void PushFrame(Handle *h);
};
void FrameReader2::PushFrame(Handle *h)
{
    Frame **pc = &cur;
    stack[depth] = *pc;
    if (++depth == 20) {
        ReportError("Overflow of previous frame stack while reading %s", name);
        ok = 0;
        return;
    }
    if (newest != h) {
        owner->v4(*pc, pc);
        int **f = h->Get();
        (*pc)->v8(*pc, **f);
    }
}
