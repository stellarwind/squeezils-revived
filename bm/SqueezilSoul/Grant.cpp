// SqueezilSoul::Grant (0x4504f0): the grant interface (second base, +4, slot 4). Self-contained: Grant needs the real
// two-base layout, so `this` arrives pointing at the +4 subobject.
#include <string.h>
#pragma intrinsic(strcmp)
#include "compat41.h"

class String {
public:
    String(const char *s);                  // 0x438380
    ~String();                              // 0x4383d0
    String &operator=(const String &o);     // 0x438670
    const char *Chars();                    // 0x438530
    char *rep;
};

struct Vector3f {
    float x, y, z;
    Vector3f(const Vector3f &o);            // 0x4183c0
    ~Vector3f();                            // 0x401660
};

struct Matrix {
    float m[4][4];
    Vector3f *Position();                   // 0x41faf0
};

void PlayEffect(String name, Vector3f at); // 0x44a250

struct SoulBody {
    virtual void b0(); virtual void b1(); virtual void b2();
    virtual Matrix *GetMatrix();            // slot 3, +0xc
};

struct Inventory {
    void Add(const char *goodie, int value);    // 0x416110
    char pad[8];
    String heldName;                        // 0x8
    int    heldValue;                       // 0xc
    int Hold(const char *goodie, int value)
    {
        if (strcmp(heldName.Chars(), "") == 0) {
            heldName = String(goodie);
            heldValue = value;
            return 1;
        }
        return 0;
    }
};

struct Possessions {
    Inventory *GetInventory();              // 0x448990
};

struct SoulBase;

struct PlaceGoodies {
    void Granted(const char *goodie, SoulBase *soul, struct Place *place);  // 0x44bc40
};

struct Place {
    PlaceGoodies *Goodies();                // 0x439b50
};

extern const char kFoodNut[];               // 0x4ba2e0
extern const char kSpitNut[];               // 0x4b9334

struct SoulBase {
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16();
    virtual SoulBody *Body();               // slot 17, +0x44
    virtual Possessions *GetPossessions();  // slot 18, +0x48
};

struct GrantMessageMap {
    virtual void g0(); virtual void g1(); virtual void g2(); virtual void g3();
    virtual int Grant(const char *goodie, int value);   // slot 4
};

struct SqueezilSoul : SoulBase, GrantMessageMap {
    char pad08[0x70 - 8];
    Place *place;                           // 0x70

    int Grant(const char *goodie, int value);
};

// @004504f0 Nuts go into their lists; any other goodie is held, if nothing is held yet. The place hears of every pickup.
int SqueezilSoul::Grant(const char *goodie, int value)
{
    PlayEffect("pickupEffect", *Body()->GetMatrix()->Position());
    if (strcmp(goodie, kFoodNut) != 0 && strcmp(goodie, kSpitNut) != 0) {
        int held = GetPossessions()->GetInventory()->Hold(goodie, value);
        if (!held)
            return 0;
    } else
        GetPossessions()->GetInventory()->Add(goodie, value);
    place->Goodies()->Granted(goodie, this, place);
    return 1;
}
