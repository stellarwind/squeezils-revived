// SqueezilRoboSoul::SqueezilRoboSoul (0x4043a0, ret 0x10): base soul constructor, then the awareness records and the AI state.
struct Vec3 { float x, y, z; Vec3() {} ~Vec3(); };             // ~Vec3 = 0x401660
struct Record { int id; Vec3 position; float distance; int stale; Record() {} ~Record() {} };   // 0x4044c0
struct Behaviour { virtual ~Behaviour(); };
struct InputTarget { virtual void i0(); };
struct Grant { virtual void g0(); };
struct SoulBase : InputTarget, Grant {
    SoulBase(int a, int b, int c);                             // 0x44f1a0
    char pad[0x78];
    virtual ~SoulBase();                                       // 0x44f600
    virtual void s1();
};
struct RoboSoul : SoulBase {
    int f80;                                                   // 0x80
    int f84;                                                   // 0x84
    char pad88[0x94 - 0x88];
    int f94;                                                   // 0x94
    int pad98;
    int f9c, fa0;                                              // 0x9c, 0xa0
    Record records[3];                                         // 0xa4
    Behaviour *behaviour;                                      // 0xec
    int mode;                                                  // 0xf0
    int arg4;                                                  // 0xf4
    RoboSoul(int a, int b, int c, int d);
    void Reset();                                              // 0x404900
    virtual ~RoboSoul();
    virtual void s1();
};
RoboSoul::RoboSoul(int a, int b, int c, int d) : SoulBase(a, b, c), f80(0), f84(1), f94(0), f9c(999), fa0(999), behaviour(0), mode(7), arg4(d)
{
    Reset();
}
