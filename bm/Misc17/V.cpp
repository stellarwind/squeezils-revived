// SQZ.EXE 0x461b50, 0x465d60, 0x479af0 candidates
struct V {
    virtual void s0(); virtual void s1();
    virtual int Slot2(int a);              // +8
    virtual void s3(); virtual void s4(); virtual void s5(); virtual void s6();
    virtual int Slot7(int a);               // +0x1c
    int F(int a);
};
int V::F(int a) {
    int r = Slot7(a); if (r != -1) Slot2(r);
}
