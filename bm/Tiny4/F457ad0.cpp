// 0x457ad0: true when the two float readings of the sub-object at +0x22c agree.
struct Gauge457 {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual float Low();        // slot 8 (+0x20)
    virtual void v9(); virtual void v10();
    virtual float High();       // slot 11 (+0x2c)
};
struct Holder457ad0 {
    char pad[0x22c];
    Gauge457 *gauge;            // 0x22c
    int Flat();
};
int Holder457ad0::Flat()
{
    return gauge->Low() == gauge->High();
}
