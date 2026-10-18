// SQZ.EXE 0x41d9e0: constructor of a polymorphic object holding a member (ctor 0x46acf0, dtor 0x46afa0) at +4 and a block of state fields.
struct Mem { Mem(); ~Mem(); char data[0x28]; };
struct C41d9e0 {
    virtual void v0();
    Mem m;                      // +4
    int f2c, f30, f34, f38, f3c, f40, f44, f48, f4c;
    C41d9e0();
};
C41d9e0::C41d9e0()
{
    f4c = 0;
    f2c = 0;
    f30 = 0;
    f34 = 0;
    f38 = 1;
    f3c = 1;
    f44 = 0;
    f48 = 0;
    f40 = 0;
}
