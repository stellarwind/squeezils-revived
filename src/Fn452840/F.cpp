struct M452840 { int d0; int d1; int d2; int d3; int d4; int d5; int d6; int d7; int d8; int d9; int d10; int d11; int d12;
    M452840(const M452840 &o) : d0(o.d0),d1(o.d1),d2(o.d2),d3(o.d3),d4(o.d4),d5(o.d5),d6(o.d6),d7(o.d7),d8(o.d8),d9(o.d9),d10(o.d10),d11(o.d11),d12(o.d12) {}
    ~M452840(); };
struct S452840 { int a, b; M452840 m; int x, y, z; S452840(const S452840 &o); };
S452840::S452840(const S452840 &o) : a(o.a), b(o.b), m(o.m), x(o.x), y(o.y), z(o.z) {}
