// 0x409620: the camera matrix part way through a blend: each axis slerped from the start matrix to the target
// matrix, the position lerped (or taken from the target when only the axes blend), then re-orthonormalised.
struct BlendVec {
    float x, y, z;
    BlendVec() {}
    BlendVec(float ax, float ay, float az) : x(ax), y(ay), z(az) {}
    ~BlendVec();                                   // 0x401660
    BlendVec operator*(float s) const;             // 0x401b60
    BlendVec operator+(const BlendVec &o) const;   // 0x403070
};
BlendVec Slerp(float t, const BlendVec &a, const BlendVec &b);   // 0x47da80
struct BlendMat {
    float m[16];
    BlendMat() {}
    BlendMat(const BlendMat &o);                   // 0x4099e0
    ~BlendMat();                                   // 0x402310
    BlendMat &SetRows(const BlendVec &right, const BlendVec &up,
                      const BlendVec &forward, const BlendVec &position);   // 0x409f00
    void Orthonormalize();                         // 0x47e350
};
struct BlendRow { BlendVec v; float w; };
struct BlendSoul {
    char pad[0x88];
    BlendRow from[4];          // 0x88
    char padc8[0x108 - 0xc8];
    BlendRow to[4];            // 0x108
    char pad148[0x14c - 0x148];
    float blendTime, blendDuration;   // 0x14c
    int blending, blendPosition;      // 0x154
    BlendMat BlendMatrix();
};
BlendMat BlendSoul::BlendMatrix()
{
    BlendMat matrix;
    BlendVec position, right, up, forward;
    float t = blendTime / blendDuration;
    if (blendPosition)
        position = from[3].v * (1.0f - t) + to[3].v * t;
    else
        position = to[3].v;
    right = Slerp(t, from[0].v, to[0].v);
    up = Slerp(t, from[1].v, to[1].v);
    forward = Slerp(t, from[2].v, to[2].v);
    if (right.x * right.x + right.y * right.y + right.z * right.z < 0.0001f)
        right = BlendVec(1.0f, 0.0f, 0.0f);
    if (up.x * up.x + up.y * up.y + up.z * up.z < 0.0001f)
        up = BlendVec(0.0f, 1.0f, 0.0f);
    if (forward.x * forward.x + forward.y * forward.y + forward.z * forward.z < 0.0001f)
        forward = BlendVec(0.0f, 0.0f, 1.0f);
    matrix.SetRows(right, up, forward, position);
    matrix.Orthonormalize();
    return matrix;
}
