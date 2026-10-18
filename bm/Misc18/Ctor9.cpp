// SQZ.EXE 0x4488d0: constructor of a polymorphic holder that allocates its 0x70-byte helper object (ctor 0x415ef0) at +4.
struct Helper { Helper(); char data[0x70]; };
struct C9 {
    virtual void v0();
    Helper *h;                  // +4
    C9();
};
C9::C9() { h = new Helper; }
