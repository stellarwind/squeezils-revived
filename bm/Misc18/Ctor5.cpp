// SQZ.EXE 0x431ad0: constructor of a class derived from the 0x422ea0 base that holds a member (ctor 0x46c7b0, dtor 0x46c7e0) at +4.
struct Mem3 { Mem3(); ~Mem3(); char data[0x10]; };
struct A5 { virtual void v0(); ~A5(); };                // dtor 0x422ea0
struct C5 : A5 {
    Mem3 m;                     // +4
    C5();
    virtual void v0();
};
C5::C5() {}
