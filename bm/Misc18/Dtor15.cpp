// SQZ.EXE 0x444800: destructor of a polymorphic holder that owns an object at +4. The owned class has an inline destructor
// (expanded in place): a base with out-of-line dtor 0x420e20 and a member at +0xc with dtor 0x4383d0.
struct ZBase { char d[0xc]; ~ZBase(); };
struct Mem2 { Mem2(); ~Mem2(); };
struct Z : ZBase {
    Mem2 m;                     // +0xc
    ~Z() {}
};
struct CB15 { virtual ~CB15(); };
struct C15 : CB15 {
    virtual ~C15();
    Z *p;                       // +4
    C15();
};
C15::C15() { p = 0; }
C15::~C15() { if (p) delete p; }
