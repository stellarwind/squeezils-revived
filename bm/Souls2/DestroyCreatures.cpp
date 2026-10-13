// LocationSimulation::DestroyCreatures (0x4141a0): standalone layout.
struct Obj { virtual ~Obj(); };

struct CreatureRec {
    void *vtbl;       // no virtual delete: plain operator delete on the record
    Obj *actor;
    Obj *soul;
};

struct CreatureTableX {
    virtual int Insert(const int &k, CreatureRec *const &v, int hint);   // 0
    virtual int Contains(const int &k);                                  // 1
    virtual int IndexOf(const int &k);                                   // 2
    virtual void RemoveAt(int i);                                        // 3
    char pad[0x24 - 4];
    CreatureRec **data;                                                  // +0x24
};

struct IdListX {
    virtual int Get(int i);          // 0
    virtual int Insert(const int &v, int at);   // 1
    virtual void RemoveAt(int i);    // 2
    int count;                       // +4
};

struct LocationSimX {
    void *vtbl;
    CreatureTableX *creatures;       // +4
    char pad[0xc - 8];
    IdListX ids;                     // +0xc (count at +0x10)
    void DestroyCreatures();
};

void LocationSimX::DestroyCreatures()
{
    if (ids.count <= 0)
        return;
    int i;
    for (i = 0; i < ids.count; i++) {
        int index = creatures->IndexOf(ids.Get(i));
        if (index == -1)
            continue;
        delete creatures->data[index]->actor;
        delete creatures->data[index]->soul;
        delete creatures->data[index];
        creatures->RemoveAt(index);
    }
    for (i = ids.count - 1; i >= 0; i--)
        ids.RemoveAt(i);
}
