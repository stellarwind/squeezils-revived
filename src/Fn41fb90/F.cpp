// Body::Body(const Mat &m) (0x41fb90, ret 4): copies the placement matrix and remembers its facing direction.
struct Mat {
    float m[16];
    Mat() {}
    ~Mat();                                              // 0x402310
    Mat &operator=(const Mat &o)
    {
        m[0] = o.m[0]; m[1] = o.m[1]; m[2] = o.m[2]; m[3] = o.m[3];
        m[4] = o.m[4]; m[5] = o.m[5]; m[6] = o.m[6]; m[7] = o.m[7];
        m[8] = o.m[8]; m[9] = o.m[9]; m[10] = o.m[10]; m[11] = o.m[11];
        m[12] = o.m[12]; m[13] = o.m[13]; m[14] = o.m[14]; m[15] = o.m[15];
        return *this;
    }
};
struct Vec { float x, y, z; Vec() {} Vec(float a, float b, float c) : x(a), y(b), z(c) {} ~Vec(); };   // 0x401660
struct Slot { int id; int data; Slot() : id(-1), data(0) {} ~Slot(); };                                // 0x4031e0
struct BodyX {
    virtual ~BodyX();
    Mat place;           // 0x04
    Vec facing;          // 0x44
    int f50;             // 0x50
    char pad[0x6c - 0x54];
    int f6c;             // 0x6c
    Slot slot;           // 0x70
    Mat scratch;         // 0x78
    BodyX(const Mat &m);
    void Reset(int a, int b, int c);   // 0x41fd60
};
BodyX::BodyX(const Mat &m)
{
    place = m;
    facing.x = place.m[8];
    facing.y = place.m[9];
    facing.z = place.m[10];
    Reset(0, 0, 0);
    f50 = 0;
    f6c = 0;
}
