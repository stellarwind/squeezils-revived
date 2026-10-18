// SQZ.EXE 0x471080: constructor of a class derived from the 0x422ea0 base holding a member (ctor 0x438370, dtor 0x4383d0) at +4 and a field at +8.
// (its scalar deleting destructor 0x4711a0 deletes the +8 object inside an EH state; not matched: 25)
struct Mem2 { Mem2(); ~Mem2(); };
struct A6 { virtual void v0(); ~A6(); };                // dtor 0x422ea0
struct C6 : A6 {
    Mem2 m;                     // +4
    int f8;
    C6();
    virtual void v0();
};
C6::C6() { f8 = 0; }

// SQZ.EXE 0x446c70: sibling with another vtable (0x49e8fc), same layout.
struct C14 : A6 {
    Mem2 m;                     // +4
    int f8;
    C14();
    virtual void v0();
};
C14::C14() { f8 = 0; }
