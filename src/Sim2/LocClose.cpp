// LocationSimulation::Close (virtual slot 7, SQZ.EXE 0x413060): destroy every entry of the list, last first.
struct LocEntryPart { virtual ~LocEntryPart(); };
struct LocEntry {
    int pad;
    LocEntryPart *a;                                     // 0x04
    LocEntryPart *b;                                     // 0x08
};
struct LocEntries {                                      // the list object (vtable slot 3 removes an index)
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual void RemoveAt(int i);                        // slot 3 (+0x0c)
    int pad[1];
    int count;                                           // 0x08
    char pad2[0x24 - 0x0c];
    LocEntry **data;                                     // 0x24
};
struct LocationSimulation {
    int pad;
    LocEntries *entries;                                 // 0x04
    void Reset();                                        // 0x4141a0
    void Close();                                        // slot 7
};

void LocationSimulation::Close()
{
    Reset();
    for (int i = entries->count - 1; i >= 0; i--) {
        LocEntry *e = entries->data[i];
        if (e->a)
            delete e->a;
        if (e->b)
            delete e->b;
        delete e;
        entries->RemoveAt(i);
    }
}
