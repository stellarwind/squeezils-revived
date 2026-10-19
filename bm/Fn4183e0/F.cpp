// SQZ.EXE 0x4183e0 (ret 8): replace the instance shown for a slot: free the old one, spawn a new one from the template for `kind`, and place it.
#define V3_CTOR_OUTOFLINE
#define V3_COPYCTOR_OUTOFLINE
#include "Vector3f/Vector3f.h"

struct Shown {
    virtual void Release(int del);                       // slot 0
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void Hide();                                 // slot 5 (+0x14)
};
struct Instance {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void Attach(int parent);                     // slot 4 (+0x10)
    virtual void s5();
    virtual void Place(int arg, Vector3f pos);           // slot 6 (+0x18)
};
struct Template {
    virtual void s0();
    virtual void s1();
    virtual Instance *Make(int parent, Vector3f pos, Vector3f scale, Vector3f forward, Vector3f up);   // slot 2 (+0x8)
};
struct Holder {
    char pad[0x24];
    int parent;                                          // 0x24
    Vector3f pos;                                        // 0x28
    Instance *shown;                                     // 0x34
    int kind;                                            // 0x38
};
struct Spawner {
    char pad[0x58];
    Template *t1;                                        // 0x58
    Template *t3;                                        // 0x5c
    Template *t0;                                        // 0x60
    Template *t2;                                        // 0x64
    void Replace(Holder *h, int kind);
};

void Spawner::Replace(Holder *h, int kind)
{
    if (h->shown) {
        ((Shown *)h->shown)->Hide();
        if (h->shown)
            ((Shown *)h->shown)->Release(1);
    }
    Template *t = 0;
    switch (kind) {
    case 0:
        t = t0;
        break;
    case 1:
        t = t1;
        break;
    case 2:
        t = t2;
        break;
    case 3:
        t = t3;
        break;
    }
    h->shown = t->Make(0, Vector3f(0, 0, 0), Vector3f(1, 1, 1), Vector3f(0, 0, 1), Vector3f(0, 1, 0));
    h->shown->Attach(h->parent);
    Instance *s = h->shown;
    s->Place(h->parent, h->pos);
    h->kind = kind;
}
