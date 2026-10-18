// SQZ.EXE 0x40b6d0: scalar deleting destructor of a two-base class (first base dtor 0x40b440) with a part at +0x18 and four 0x40-byte members (dtor 0x402310).
struct Part { int v[3]; Part() {} ~Part(); };
struct M { M(); ~M(); char d[0x40]; };
struct A40 { virtual ~A40(); };
struct B40 { virtual void g(); char d[0x10]; };
struct D40 : A40, B40 {
    Part p;                     // +0x18
    char pad24[0x24];
    M m48, m88, mc8, m108;
    D40();
    virtual ~D40() {}
};
D40::D40() {}
