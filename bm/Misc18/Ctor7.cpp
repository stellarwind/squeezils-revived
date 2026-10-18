// SQZ.EXE 0x416e50: constructor taking two values; base ctor 0x412e80 (dtor 0x412f20), member at +0x30 (ctor 0x4690c0, dtor 0x469900).
struct Base7 { Base7(); virtual ~Base7(); char data[0x20]; };
struct Mem7 { Mem7(); ~Mem7(); char data[0x10]; };
struct C7 : Base7 {
    int f24, f28;               // +0x24, +0x28
    char pad2c[4];
    Mem7 m;                     // +0x30
    C7(int a, int b);
    virtual void v0();
};
C7::C7(int a, int b)
{
    f24 = a;
    f28 = b;
}
