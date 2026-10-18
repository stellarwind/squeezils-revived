// SQZ.EXE 0x441350: destructor of a holder (base dtor 0x422ea0) that owns three objects at +4/+8/+0xc; each owned class has an inline
// destructor (base dtor 0x420e20 plus one member at +0xc).
struct ZBase { char d[0xc]; ~ZBase(); };
struct Mem2 { Mem2(); ~Mem2(); };
struct Part { int v[3]; Part() {} ~Part(); };
struct Z1 : ZBase { Mem2 m; ~Z1() {} };
struct Z2 : ZBase { Part m; ~Z2() {} };
struct CB16 { virtual ~CB16(); };
struct C16 : CB16 {
    virtual ~C16();
    Z1 *a;                      // +4
    Z2 *b;                      // +8
    Z2 *c;                      // +0xc
    C16();
};
C16::C16() { a = 0; b = 0; c = 0; }
C16::~C16()
{
    delete a;
    delete b;
    delete c;
}
