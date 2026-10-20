// Constructor of a 0x320-byte object derived from a base (ctor 0x47a8f0) taking the base's three arguments plus three of its own (SQZ.EXE 0x479b10).
struct Base2 { Base2(int a, int b, int c); char data[0x308]; virtual void v0(); };   // ctor = 0x47a8f0
struct Derived2 : Base2 {
    int f0, f1, f2, f3, f4;     // +0x30c..+0x31c
    Derived2(int a, int b, int c, int d, int e, int f);
    virtual void v0();
};
Derived2::Derived2(int a, int b, int c, int d, int e, int f) : Base2(a, b, c)
{
    f0 = d;
    f1 = e;
    f2 = f;
    f3 = f;
    f4 = 0;
}
