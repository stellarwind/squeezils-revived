// SQZ.EXE 0x446420: constructor of a three-level class (bases with out-of-line destructors) that holds a member at +8.
struct Mem2 { Mem2(); ~Mem2(); char data[0x10]; };
struct A4 { virtual void v0(); ~A4(); };                // dtor 0x422ea0
struct B4 : A4 { int f4; B4() { f4 = 0; } ~B4(); virtual void v0(); };   // dtor 0x4439a0
struct C4 : B4 {
    Mem2 m;                     // +8
    C4();
    virtual void v0();
};
C4::C4() {}
