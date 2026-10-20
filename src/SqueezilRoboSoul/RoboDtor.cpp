// SqueezilRoboSoul destructor body (SQZ.EXE 0x404510): delete the behaviour, destroy the three awareness records, then the base (0x44f600).
struct Vec3 { float x, y, z; Vec3() {} ~Vec3(); };             // ~Vec3 = 0x401660
struct Record { int id; Vec3 position; float distance; int stale; Record() {} ~Record() {} };   // 0x4044c0
struct Behaviour { virtual ~Behaviour(); };
struct InputTarget { virtual void i0(); };
struct Grant { virtual void g0(); };
struct SoulBase : InputTarget, Grant {
    char pad[0x78];
    virtual ~SoulBase();                                       // 0x44f600
    virtual void s1();
};
struct RoboSoul : SoulBase {
    char pad2[0x24];
    Record records[3];                                         // 0xa4
    Behaviour *behaviour;                                      // 0xec
    char rest[0xc];
    virtual ~RoboSoul();
    virtual void s1();
};

RoboSoul::~RoboSoul()
{
    delete behaviour;
}
