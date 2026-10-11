// 0x4099e0 (second 16 word copy), 0x414250 NewId, 0x414010, 0x414380 (zeroing ctor), 0x410890 (two set lookups).
// one spare declaration (symbol count, see padscan.sh)
void PadDecl1();
struct Mi16b {
    int m[16];
    Mi16b *Assign(const int *src);
};

Mi16b *Mi16b::Assign(const int *src)
{
    m[0] = src[0];
    m[1] = src[1];
    m[2] = src[2];
    m[3] = src[3];
    m[4] = src[4];
    m[5] = src[5];
    m[6] = src[6];
    m[7] = src[7];
    m[8] = src[8];
    m[9] = src[9];
    m[10] = src[10];
    m[11] = src[11];
    m[12] = src[12];
    m[13] = src[13];
    m[14] = src[14];
    m[15] = src[15];
    return this;
}

struct CTab {
    virtual int Insert(const int &k, void *v, int hint);   // 0
    virtual int Contains(const int &k);                    // 1
    char pad[0x8 - 4];
    int count;                                             // +8 (keys.count)
    char pad2[0x24 - 0xc];
    struct CRec **data;                                    // +0x24
};
struct CAct { virtual void s0(); virtual void s1(); virtual void Touch(); };
struct CRec { void *vtbl; CAct *actor; void *soul; };

struct LocSim {
    void *vtbl;
    CTab *creatures;    // +4
    int lastId;         // +8
    int NewId();
    void TouchActors();
    void ResetVtbl();
};

void LocSim::ResetVtbl()
{
    vtbl = (void *)0x49d644;
}

int LocSim::NewId()
{
    do {
        lastId++;
    } while (creatures->Contains(lastId));
    return lastId;
}

void LocSim::TouchActors()
{
    for (int i = 0; i < creatures->count; i++)
        creatures->data[i]->actor->Touch();
}

struct Zeroed {
    int a[2];
    int d[2];
    int e[2];
    int c[2];
    int b[2];
    int f28, f2c;
    char pad[0x64 - 0x30];
    int f64, f68, f6c, f70;
    Zeroed *Init();
};

Zeroed *Zeroed::Init()
{
    for (int i = 0; i < 2; i++) {
        a[i] = 0;
        d[i] = 0;
        b[i] = 0;
        c[i] = 0;
        e[i] = 0;
    }
    f28 = 0;
    f2c = 0;
    f64 = 0;
    f68 = 0;
    f70 = 0;
    f6c = 0;
    return this;
}

struct SetObj {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5();
    virtual int Contains(int v);   // 6, +0x18
    int body[3];
};
struct TwoSets {
    char pad[0x144];
    SetObj first;                    // +0x144
    char pad2[0x158 - 0x144 - sizeof(SetObj)];
    SetObj second;                   // +0x158
    int InEither(int v);
};

int TwoSets::InEither(int v)
{
    if (second.Contains(v) || first.Contains(v))
        return 1;
    return 0;
}
