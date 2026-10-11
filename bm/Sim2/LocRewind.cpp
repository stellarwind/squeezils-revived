// LocationSimulation slot 3 (SQZ.EXE 0x413f80): start the b-parts (all, with entry 1 last), stop every a-part, reset.
struct LocPart {
    virtual void v0();
    virtual void Stop();                                 // slot 1 (+4)
    virtual void v2(); virtual void v3(); virtual void v4();
    virtual void Start();                                // slot 5 (+0x14)
};
struct LocEntry {
    int pad;
    LocPart *a;                                          // 0x04
    LocPart *b;                                          // 0x08
};
struct LocEntries {
    char pad[8];
    int count;                                           // 0x08
    char pad2[0x24 - 0x0c];
    LocEntry **data;                                     // 0x24
};
struct LocationSimulation {
    int pad;
    LocEntries *entries;                                 // 0x04
    void Reset();                                        // 0x4141a0
    void Rewind();                                       // slot 3
};

void LocationSimulation::Rewind()
{
    entries->data[0]->b->Start();
    for (int i = 2; i < entries->count; i++)
        entries->data[i]->b->Start();
    entries->data[1]->b->Start();
    for (int j = 0; j < entries->count; j++)
        entries->data[j]->a->Stop();
    Reset();
}
