// SQZ.EXE 0x446780: scalar deleting destructor of a class (base dtor 0x4464b0) that owns an object at +0xc whose class has an inline destructor
// (only its base, dtor 0x420e20).
struct ZBase { char d[0xc]; ~ZBase(); };
struct Z0 : ZBase { ~Z0() {} };
struct B18 { virtual ~B18(); int a, b; };
struct C18 : B18 {
    Z0 *p;                      // +0xc
    C18();
    virtual ~C18() { delete p; }
};
C18::C18() { p = 0; }
