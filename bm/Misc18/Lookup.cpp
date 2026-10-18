// SQZ.EXE 0x40f3c0: look an item up by id in the list at +0x134 and ask its record's sub-object for a Part, passing a helper's result.
struct Part { int v[3]; Part() {} ~Part(); };
struct Sub { virtual Part S0(int h); };
struct Rec { char pad[0x14]; Sub sub; };
struct Item3 { Rec *Ptr(); };                       // 0x40c6b0
struct ListC { virtual Item3 *Get(int id); };
struct Holder6 {
    char pad[0x134];
    ListC c;                                        // +0x134
    int Helper(int *key);                           // 0x40f380
    Part Lookup(int *key);
};
Part Holder6::Lookup(int *key)
{
    int h = Helper(key);
    return c.Get(*key)->Ptr()->sub.S0(h);
}
