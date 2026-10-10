// The complete-object-destructor body of a class with a virtual dtor, one non-trivial member at a
// specific offset, and a base class with a virtual dtor (SQZ.EXE). The vtable address and both dtor
// calls are masked, so only the member offset distinguishes the six instantiations.
struct TMember {
    ~TMember();
};
struct TBase {
    virtual ~TBase();
};

struct D1 : TBase { char pad[0xb0 - 4]; TMember m; virtual ~D1(); };  // LODProp
struct D2 : TBase { char pad[0xac - 4]; TMember m; virtual ~D2(); };  // CycleProp
struct D3 : TBase { char pad[0x30 - 4]; TMember m; virtual ~D3(); };  // RegionSimulation
struct D4 : TBase { char pad[0x0c - 4]; TMember m; virtual ~D4(); };  // LocationSimulation
struct D5 : TBase { char pad[0x40 - 4]; TMember m; virtual ~D5(); };  // MeshProp
struct D6 : TBase { char pad[0xfc - 4]; TMember m; virtual ~D6(); };  // NavProp
struct D7 : TBase { virtual ~D7(); };                                 // PlaceSimulation (no member)

struct TStr {
    void *rep;       // 4 bytes (a String handle)
    ~TStr();
};
struct TSettings {
    ~TSettings();
};
struct D8 : TBase { char pad[0x40 - 4]; TStr name; TSettings settings; virtual ~D8(); };  // SpriteProp (2 members)

struct Vec3 {
    float x, y, z;
    ~Vec3();
};
struct D9 {           // root class (no base): vtbl at +0, members from +0x04
    Vec3 position;    // 0x04
    Vec3 scale;       // 0x10
    Vec3 direction;   // 0x1c
    Vec3 up;          // 0x28
    void Hide();      // 0x426170
    virtual ~D9();
};

D1::~D1() {}
D2::~D2() {}
D3::~D3() {}
D4::~D4() {}
D5::~D5() {}
D6::~D6() {}
D7::~D7() {}
D8::~D8() {}
D9::~D9() { Hide(); }
