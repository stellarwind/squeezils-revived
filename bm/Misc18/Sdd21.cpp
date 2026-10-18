// SQZ.EXE 0x4596e0: compiler-generated scalar deleting destructor of a two-base class: second base at +4 (dtor 0x450980), member at +0xc
// (dtor 0x4383d0) and a part at +0x20.
struct Part { int v[3]; Part() {} ~Part(); };
struct Mem2 { Mem2(); ~Mem2(); char d[0x14]; };
struct A21 { virtual ~A21() {} };
struct B21 { char d[8]; ~B21(); };                       // dtor 0x450980
struct D21 : A21, B21 {
    Mem2 m;                     // +0xc
    Part p;                     // +0x20
    D21();
};
D21::D21() {}
