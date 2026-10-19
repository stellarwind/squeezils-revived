// Two constructors of a small reference holder (SQZ.EXE 0x4207a0 ret 8, 0x420900 ret 0xc): use the given handle, else look it up through the creature's placement.
struct RefMat {                                          // 64 bytes, empty out-of-line destructor 0x402310
    float a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15;
    RefMat() {}
    RefMat(const RefMat &o)
        : a0(o.a0), a1(o.a1), a2(o.a2), a3(o.a3),
          a4(o.a4), a5(o.a5), a6(o.a6), a7(o.a7),
          a8(o.a8), a9(o.a9), a10(o.a10), a11(o.a11),
          a12(o.a12), a13(o.a13), a14(o.a14), a15(o.a15) {}
    ~RefMat();
};
struct RefPlacement {                                    // 0x48 bytes, destructor 0x4031e0
    int prop;
    int pad;
    RefMat at;
    RefPlacement() {}
    RefPlacement(const RefPlacement &o) : prop(o.prop), pad(o.pad), at(o.at) {}
    ~RefPlacement();
};
struct RefCreature { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
                     virtual void v6(); virtual RefPlacement &Placement(); };        // slot 7 (+0x1c)
struct RefIndex { int Resolve(RefPlacement *p); };       // 0x40c6c0
struct RefLevel { RefIndex *Index(); };                  // 0x439b60
struct RefPlace { RefLevel *GetLevel(); };
struct RefRoot { RefCreature *GetCreature(int id); };   // 0x414050
RefRoot *GetSim();                                       // 0x420690 (cdecl)
void *DynamicCast(void *p, int zero, const void *from, const void *to, int ref);   // 0x48f060 (cdecl)
extern const char kSimType[];                            // 0x4ba1d0
extern const char kPlaceSimType[];                       // 0x4bae38

struct RefHolder {
    int f0, f4, f8, fc, id, handle;
    RefHolder(int id, int handle);
};
struct RefHolder4 {
    int f0, f4, f8, fc, id, handle;
    RefHolder4(int a, int id, int handle);
};
struct RefHolder3 {
    int f0, f4, f8, fc, id, handle;
    RefHolder3(int a, int id, int handle);
};

RefHolder::RefHolder(int id_, int handle_)
{
    if (!handle_) {
        RefPlacement p(GetSim()->GetCreature(id_)->Placement());
        if (p.prop != -1) {
            handle = ((RefPlace *)DynamicCast(GetSim(), 0, kSimType, kPlaceSimType, 0))->GetLevel()->Index()->Resolve(&p);
        } else
            handle = 0;
    } else
        handle = handle_;
    f8 = 0;
    fc = 0;
    f0 = 0;
    f4 = 0;
    id = id_;
}

RefHolder3::RefHolder3(int a, int id_, int handle_)
{
    if (!handle_) {
        RefPlacement p(GetSim()->GetCreature(id_)->Placement());
        if (p.prop != -1) {
            handle = ((RefPlace *)DynamicCast(GetSim(), 0, kSimType, kPlaceSimType, 0))->GetLevel()->Index()->Resolve(&p);
        } else
            handle = 0;
    } else
        handle = handle_;
    f8 = a;
    fc = 0;
    f0 = 0;
    f4 = 0;
    id = id_;
}

RefHolder4::RefHolder4(int a, int id_, int handle_)
{
    if (!handle_) {
        RefPlacement p(GetSim()->GetCreature(id_)->Placement());
        if (p.prop != -1) {
            handle = ((RefPlace *)DynamicCast(GetSim(), 0, kSimType, kPlaceSimType, 0))->GetLevel()->Index()->Resolve(&p);
        } else
            handle = 0;
    } else
        handle = handle_;
    f8 = a;
    f0 = a;
    fc = 0;
    id = id_;
}
