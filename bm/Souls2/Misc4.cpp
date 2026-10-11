// 0x44e020 (clear a field in every matching element) and 0x44ef00 (call slot 1 as many times as the counter says).
void PadDecl1();

struct ElemE { char pad[0x48]; int f48; char pad2[0x70 - 0x4c]; int f70; };
struct ElemArr {
    virtual ElemE *Get(int i);   // 0
    int count;                   // +4
};
struct Owner {
    char pad[0x1e8];
    ElemArr arr;                 // +0x1e8
    void ClearMatching(int key);
};

void Owner::ClearMatching(int key)
{
    for (int i = 0; i < arr.count; i++) {
        if (arr.Get(i)->f70 == key)
            arr.Get(i)->f48 = 0;
    }
}

struct Pumper {
    virtual void s0();
    virtual void Step();          // 1
    int pad1, pad2;
    int pending;                  // +0xc
    void Drain();
};

void Pumper::Drain()
{
    int n = pending;
    pending = 0;
    for (; n > 0; n--)
        Step();
}

struct Progress {
    virtual void s0();
    virtual void Step();          // 1, +4
    int pad1, pad2;
    int done;                     // +0xc
    int total;                    // +0x10
    char pad3[0x60 - 0x14];
    float shown;                  // +0x60
    void SetTarget(float t);
};

void Progress::SetTarget(float t)
{
    if (shown < t || *(int *)&shown >= 0x3f800000) {
        if (*(int *)&shown != 0x3f800000) {
            if ((float)done / (float)total < shown) {
                do {
                    Step();
                } while ((float)done / (float)total < shown);
            }
        }
        shown = t;
    }
    Step();
}
