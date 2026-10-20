// SQZ.EXE 0x471ab0: constructor taking the base ctor's three arguments (0x47a8f0) plus two own fields at +0x30c/+0x310.
struct Base12 { Base12(int a, int b, int c); char data[0x308]; virtual void v0(); };
struct C12 : Base12 {
    int f0, f1;                 // +0x30c, +0x310
    C12(int a, int b, int c, int d, int e);
    virtual void v0();
};
C12::C12(int a, int b, int c, int d, int e) : Base12(a, b, c)
{
    f0 = d;
    f1 = e;
}
