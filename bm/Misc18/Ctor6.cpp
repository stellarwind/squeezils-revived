// SQZ.EXE 0x471080: constructor of a class derived from the 0x422ea0 base holding a member (ctor 0x438370, dtor 0x4383d0) at +4 and a field at +8.
// Its destructor 0x4711a0 does `if (f8) delete f8;` with f8 an object that has an inline dtor (see Dtor15.cpp) but one EH state fewer than
// the obvious source gives (best 35); not matched. 0x446c70 is a sibling constructor with another vtable.
struct Mem2 { Mem2(); ~Mem2(); };
struct A6 { virtual void v0(); ~A6(); };                // dtor 0x422ea0
struct C6 : A6 {
    Mem2 m;                     // +4
    int f8;
    C6();
    virtual void v0();
};
C6::C6() { f8 = 0; }

struct C14 : A6 {
    Mem2 m;                     // +4
    int f8;
    C14();
    virtual void v0();
};
C14::C14() { f8 = 0; }
