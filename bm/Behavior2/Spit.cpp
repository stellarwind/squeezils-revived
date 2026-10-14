// SpitBehavior::Execute (0x403d90): aim at the tracked creature, wait until it is close, spit when holding spit, cool down.
#include "Behavior2/Activity.h"
#include "Vector3f/Vector3f.h"

struct SpitSub {
    virtual int Run(int a);                              // slot 0
    virtual int Execute(int a);                          // slot 1
    char pad[0x2b4 - 4];
    unsigned flags;                                      // 0x2b4
    int mode;                                            // 0x2b8
    Vector3f dest;                                       // 0x2bc
};
struct SpitInventory { int Count(const char *kind); };    // 0x416300
struct SpitOwner { SpitInventory *Inventory(); };        // 0x448990
struct SpitRecord {
    int id;                                              // 0x00
    Vector3f pos;                                        // 0x04
    float weight;                                        // 0x10
    int pad14;
};
struct SpitSoul {
    char pad[0xa0 - 4];
    int index;                                           // 0xa0
    SpitRecord records[1];                               // 0xa4
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16(); virtual void v17();
    virtual SpitOwner *Owner();                           // slot 18 (+0x48)
    virtual void v19(); virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25(); virtual void v26(); virtual void v27(); virtual void v28();
    virtual int Spit();                                  // slot 29 (+0x74)
    virtual void Cool(float t);                          // slot 30 (+0x78)
};
struct SpitClock { int Elapsed(); };                     // 0x41ec00
SpitClock *GetClock();                                   // 0x4206d0 (cdecl)
extern const char kSpitName[];                           // 0x4b9334

struct SpitBehavior {
    virtual void s0();
    virtual int Execute(int a, int b);                   // slot 1
    void *body;                                          // 0x04
    SpitSoul *soul;                                      // 0x08
    SpitSub *current;                                    // 0x0c
    int mode;                                            // 0x10
    SpitSub *aim;                                        // 0x14
    int pad18;
    int countdown;                                       // 0x1c
    float closeEnough;                                          // 0x20
    float delay;                                         // 0x24
};

int SpitBehavior::Execute(int a, int b)
{
    SpitRecord *rec = &soul->records[soul->index];
    int result = 0x18;
    SpitSub *s = aim;
    s->dest.x = rec->pos.x;
    s->dest.y = rec->pos.y;
    s->dest.z = rec->pos.z;
    switch (mode) {
    case 0:
        current = aim;
        aim->mode = 0;
        mode++;
        break;
    case 1:
        if (rec->weight < closeEnough)
            mode++;
        break;
    case 2:
        if (soul->Owner()->Inventory()->Count(kSpitName) > 0) {
            result = soul->Spit();
            mode++;
        }
        break;
    case 3:
        if (!(aim->flags & 0x7fffffff)) {
            soul->Cool(1.0f);
            countdown = (int)(delay * 1000.0f);
            mode++;
        }
        break;
    case 4:
        countdown -= GetClock()->Elapsed();
        if (countdown <= 0)
            mode = 0;
        break;
    }
    int r = current->Execute(a);
    if (result != 0x18)
        r = result;
    return r;
}
