// 0x436fc0 (cdecl): a flat quad of half-width gRegionBridgeWidth from a to b, coloured (r, g, b).
struct BridgeVec {
    float x, y, z;
    BridgeVec() {}
    BridgeVec(float ax, float ay, float az) : x(ax), y(ay), z(az) {}
    BridgeVec(const BridgeVec &o);                          // 0x4183c0
    ~BridgeVec() {}
    BridgeVec &operator=(const BridgeVec &o) { x = o.x; y = o.y; z = o.z; return *this; }
    BridgeVec operator-(const BridgeVec &o) const;          // 0x401ae0
    BridgeVec operator*(float s) const;                     // 0x401b60
    BridgeVec operator+(const BridgeVec &o) const;          // 0x403070
    BridgeVec Cross(const BridgeVec &o) const;              // 0x437360
    float Normalize();                                      // 0x47d630
};
struct BridgeFrame;
struct BridgeScene {
    BridgeFrame *CreateQuad(BridgeVec a, BridgeVec b, BridgeVec c, BridgeVec d, BridgeVec normal,
        double r, double g, double blue);                   // 0x4777b0
};
extern BridgeScene *g_scene;                                // 0x4c75e0
extern float gRegionBridgeWidth;                            // 0x4c7338

BridgeFrame *__stdcall MakeBridgeQuad(BridgeVec a, BridgeVec b, float r, float g, float blue)
{
    BridgeVec a1;
    a1 = a;
    BridgeVec a2;
    a2 = a1;
    BridgeVec b1;
    b1 = b;
    BridgeVec b2;
    b2 = b1;
    BridgeVec dir = b - a;
    BridgeVec side = dir.Cross(BridgeVec(0.0f, 1.0f, 0.0f));
    side.Normalize();
    side = side * gRegionBridgeWidth;
    a1 = a1 - side;
    a2 = a2 + side;
    b1 = b1 + side;
    b2 = b2 - side;
    BridgeVec normal = side.Cross(dir);
    normal.Normalize();
    return g_scene->CreateQuad(a1, a2, b1, b2, normal, r, g, blue);
}
