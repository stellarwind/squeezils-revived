// Small accessors that forward to an embedded container object (SQZ.EXE 0x40c6e0, 0x4102f0, 0x410310, 0x412bc0).
struct Item { int pad; };
struct ListA {                                           // object at +0xc8
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual void Slot3(int a, const int &b);             // +0xc
};
struct ListB {                                           // object at +0x144
    virtual void s0();
    virtual void Insert(const int &v, int at);           // +4
};
struct ListC {                                           // object at +0x134
    virtual Item *Get(int id);                           // +0
};
struct Item2 { void Apply(int x); };                     // 0x40c470
struct Item3 { int *Ptr(); };                            // 0x40c6b0
struct Holder {
    char pad[0xc8];
    ListA a;                                             // 0xc8
    char pad2[0x134 - 0xcc];
    ListC c;                                             // 0x134
    char pad3[0x144 - 0x138];
    ListB b;                                             // 0x144
    void Forward1(int x, int y);                         // 0x40c6e0 (ret 8)
    void Forward2(int v);                                // 0x4102f0 (ret 4)
    void Forward3(int id, int x);                        // 0x410310 (ret 8)
    int Forward4(int *id);                               // 0x412bc0 (ret 4)
};

void Holder::Forward1(int x, int y) { a.Slot3(x, y); }
void Holder::Forward2(int v) { b.Insert(v, -1); }
void Holder::Forward3(int id, int x) { Item *it = c.Get(id); ((Item2 *)it)->Apply(x); }
int Holder::Forward4(int *id) { return *((Item3 *)c.Get(*id))->Ptr(); }
