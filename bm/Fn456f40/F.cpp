// DirectionHistory::Add (0x456f40, ret 4): stores a direction in the ring and keeps a running sum of the ring's contents.
struct V3 {
    float x, y, z;
    V3() {}
    ~V3();                                           // 0x401660
    V3 operator+(const V3 &o) const;                 // 0x403070
    V3 operator-(const V3 &o) const;                 // 0x401ae0
    V3 &operator=(const V3 &o) { x = o.x; y = o.y; z = o.z; return *this; }
};
struct DirectionHistory {
    int size;            // 0x00
    V3 *items;           // 0x04
    V3 sum;              // 0x08
    int index;           // 0x14
    int filled;          // 0x18
    V3 *Add(const V3 &v);
};
V3 *DirectionHistory::Add(const V3 &v)
{
    items[index] = v;
    int next = index + 1;
    if (next == size)
        next = 0;
    sum = (sum + v) - items[next];
    index = next;
    filled++;
    return &sum;
}
