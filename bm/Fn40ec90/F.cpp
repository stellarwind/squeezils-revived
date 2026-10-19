// 0x40ec90: deletes every item of the list at +0x134 (from the back) and removes it from the list.
struct Vec { float x, y, z; ~Vec(); };               // 0x401660
struct MatT { char d[0x40]; ~MatT(); };              // 0x402310
struct PartX { char d[0x14]; ~PartX(); };            // 0x40c420
struct PartY { char d[0x30]; ~PartY(); };            // 0x40c3d0
struct Item {
    char pad[4];
    MatT a;                                          // 0x04
    char pad2[0x48 - 0x44];
    MatT b;                                          // 0x48
    char pad3[0x98 - 0x88];
    PartY y;                                         // 0x98
    PartX x;                                         // 0xc8
    Vec v1;                                          // 0xdc
    Vec v2;                                          // 0xe8
    ~Item() {}
};
struct List { virtual Item *Get(int i); virtual int v1(); virtual void Remove(int i); int count; };
struct Owner {
    char pad[0x134];
    List items;                                      // 0x134
    void Clear();
};
void Owner::Clear()
{
    for (int i = items.count - 1; i >= 0; i--) {
        if (items.Get(i)) {
            Item *it = items.Get(i);
            if (it)
                delete it;
        }
        items.Remove(i);
    }
}
