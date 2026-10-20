// SQZ.EXE 0x436e10 (ret 8, this = the secondary base at +0x80): on message 0x18 take the place number from the text, and move the active input's matrix to it.
#include <string.h>
#include <stdlib.h>
#define V3_COPYCTOR
#include "Vector3f/Vector3f.h"

struct PMat {                                            // 64 bytes, empty out-of-line destructor 0x402310
    float v[16];
    PMat() {}
    PMat(const PMat &o)
    {
        v[0] = o.v[0]; v[1] = o.v[1]; v[2] = o.v[2]; v[3] = o.v[3];
        v[4] = o.v[4]; v[5] = o.v[5]; v[6] = o.v[6]; v[7] = o.v[7];
        v[8] = o.v[8]; v[9] = o.v[9]; v[10] = o.v[10]; v[11] = o.v[11];
        v[12] = o.v[12]; v[13] = o.v[13]; v[14] = o.v[14]; v[15] = o.v[15];
    }
    ~PMat();
    void Move(const Vector3f &to);                       // 0x47e330
};
struct PInput {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual PMat *GetMatrix();                           // slot 3 (+0xc)
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
    virtual void SetMatrix(PMat *m);                     // slot 9 (+0x24)
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual int IsActive();                              // slot 15 (+0x3c)
};
struct PPlace { Vector3f pos; };
struct PRegion {
    int HasPlace(int id);                                // 0x418120
    PPlace *GetPlace(int id);                            // 0x418140
    void Select(int id);                                 // 0x418170
};
struct PPrimary {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual PInput *GetInput();      // slot 17 (+0x44)
};
struct PSecond {
    char pad0[8];
    PRegion *region;                                     // 0x08
    char pad1[0x10 - 0xc];
    int place;                                           // 0x10
    char pad2[0x34 - 0x14];
    int flag;                                            // 0x34
    char text[8];                                        // 0x38
    int length;                                          // 0x40
    void Message(int id, const char *s);
};

void PSecond::Message(int id, const char *s)
{
    if (id != 0x18)
        return;
    flag = 0;
    strcpy(text, s);
    length = strlen(text);
    if (!((PPrimary *)((char *)this - 0x80))->GetInput()->IsActive())
        return;
    int n = atoi(text);
    if (!region->HasPlace(n))
        return;
    place = n;
    PMat m = *((PPrimary *)((char *)this - 0x80))->GetInput()->GetMatrix();
    Vector3f v = region->GetPlace(place)->pos;
    m.Move(v);
    ((PPrimary *)((char *)this - 0x80))->GetInput()->SetMatrix(&m);
    region->Select(place);
}
