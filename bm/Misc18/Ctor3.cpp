// SQZ.EXE 0x41d9e0 (constructor) and 0x41dc70 (destructor): a polymorphic object holding a member (ctor 0x46acf0, dtor 0x46afa0) at +4
// and a block of state fields; the destructor deletes the items of the array at +0x28 (count at +0xc) and the objects at +0x40/+0x44.
struct Item { ~Item(); };                    // 0x41d8e0
struct Cam { ~Cam(); };                      // 0x41b5f0
struct Mem { Mem(); ~Mem(); int d[10]; };    // d[2] = count (+0xc), d[9] = Item** (+0x28)
struct C41d9e0 {
    virtual ~C41d9e0();
    Mem m;                      // +4
    int f2c, f30, f34, f38, f3c;
    Cam *f40;
    Cam *f44;
    int f48, f4c;
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
C41d9e0::~C41d9e0()
{
    for (int i = 0; i < m.d[2]; i++)
        delete ((Item **)m.d[9])[i];
    delete f40;
    delete f44;
}
