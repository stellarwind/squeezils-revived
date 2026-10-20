// 0x425c00: returns a copy of the global 0x2c-byte record at 0x4c7190 (three embedded vectors).
struct V3 { float x, y, z; V3() {} V3(const V3 &o) : x(o.x), y(o.y), z(o.z) {} ~V3(); };   // dtor 0x401660
struct Big {
    int a;          // 0x00
    V3 b;           // 0x04
    V3 c;           // 0x10
    int d;          // 0x1c
    V3 e;           // 0x20
    Big();
    Big(const Big &o) : a(o.a), b(o.b), c(o.c), d(o.d), e(o.e) {}
    ~Big();         // 0x425a30
};
Big::Big() {}
extern Big g_big;                                   // 0x4c7190
Big GetBig()
{
    return g_big;
}
