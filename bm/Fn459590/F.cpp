// GoodieSoul constructor (SQZ.EXE 0x459590, ret 0xc): the PhysicalSoul part (0x446e30), the GrantMessageMap base at +4, then the goodie's own fields.
#include "String/String.h"
#include "Vector3f/Vector3f.h"
struct MMap { virtual ~MMap(); };                                  // vtable 0x49d2b4
struct GrantMessageMap : MMap { virtual void Grant(); };           // 0x49f844
struct PhysicalSoul {
    virtual ~PhysicalSoul();
    PhysicalSoul();                                                // 0x446e30
};
struct GoodieBody { char pad[0x6c]; int field6c; char pad2[0xbc - 0x70]; int fieldbc; };
struct GoodieSettings { char pad[0x30]; int field30; };
extern GoodieSettings *g_goodieSettings;                           // 0x4c7bb4
extern int g_goodieField;                                          // 0x4c7bb8
extern GoodieSettings g_goodieStorage;                             // 0x4c7bc0
extern int g_goodieSettingsRead;                                   // 0x4bf354

struct GoodieSoulC : PhysicalSoul, GrantMessageMap {
    GoodieBody *body;                                              // 0x08
    String name;                                                   // 0x0c
    int kind;                                                      // 0x10
    int thrown;                                                    // 0x14
    int f18;                                                       // 0x18
    char pad1c[4];
    Vector3f vel;                                                  // 0x20
    float f2c;                                                     // 0x2c
    int f30;                                                       // 0x30
    void ReadSettings();                                           // 0x4597b0
    GoodieSoulC(GoodieBody *b, const char *n, int k);
    virtual void Grant();
};

GoodieSoulC::GoodieSoulC(GoodieBody *b, const char *n, int k)
{
    body = b;
    name = String(n);
    if (!g_goodieSettingsRead) {
        g_goodieSettings = &g_goodieStorage;
        ReadSettings();
    }
    b->field6c = g_goodieField;
    b->fieldbc = g_goodieSettings->field30;
    vel.x = 0;
    vel.y = 0;
    vel.z = 0;
    thrown = 0;
    f18 = 0;
    f30 = 0;
    f2c = -1.0f;
    kind = k;
}
