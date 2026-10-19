// LandSimulation::Cleanup (0x41e700): deletes the possessions that are not lent out, then the table and the lent-id set.
struct Poss { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void Destroy(int del); };
struct Keys { virtual int Get(int i); };
struct TableCore {                                           // dtor 0x4651d0
    virtual void t0();
    Keys keys;                                                 // 0x04
    int count;                                                 // 0x08
    char pad[0x24 - 0x0c];
    Poss **values;                                             // 0x24
    ~TableCore();
};
struct Table7 { TableCore core; ~Table7() {} };
struct Set10 { char d[0x1c]; ~Set10(); };                      // 0x45efa0
class LandSim {
public:
    virtual ~LandSim();
    virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4();
    virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8();
    virtual int IsLentOut(int id);                             // slot 9
    void *bufVtbl;                                             // 0x04 (BufferObject)
    Table7 *table;                                             // 0x08
    Set10 *lentIds;                                            // 0x0c
    void Cleanup();
};
void LandSim::Cleanup()
{
    for (int i = 0; i < table->core.count; i++) {
        Poss *p = table->core.values[i];
        if (!IsLentOut(table->core.keys.Get(i)) && p)
            p->Destroy(1);
    }
    delete table;
    table = 0;
    delete lentIds;
    lentIds = 0;
}
