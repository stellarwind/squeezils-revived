// SqueezilSoul: nuts (0x450740).
// 3 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
#include "Soul.h"

struct DropMatrix {                         // a body matrix passed by value (64 bytes)
    float m[16];
    DropMatrix(const Matrix &o);            // 0x4099e0
    ~DropMatrix();                          // empty, out of line (0x402310)
};

struct NutList {
    virtual int At(int i);                  // slot 0
};

struct NutInventory {
    int      Count(const char *kind);                  // 0x416300
    NutList *Items(const char *kind);                  // 0x4163d0
    void     Remove(const char *kind, int index);      // 0x416280
};

struct PlaceGoodies {
    void Drop(int item, const char *kind, DropMatrix at);   // 0x44d4f0
};

struct GoodiePlace {
    PlaceGoodies *Goodies();                // 0x439b50
};

extern const char kFoodNut[];               // 0x4ba2e0
extern const char kSpitNut[];               // 0x4b9334
extern float gFoodNutSprayPercent;          // 0x4c7820
extern float gSpitNutSprayPercent;          // 0x4c7b0c

// @00450740 On a hit the squeezil loses a share of each kind of nut (at least one), dropped where it stands.
void SqueezilSoul::SprayNuts()
{
    NutInventory *inv = (NutInventory *)GetPossessions()->GetInventory();
    int count;
    count = inv->Count(kFoodNut);
    if (count) {
        int drop = count > 1 ? (int)(count * gFoodNutSprayPercent) : 1;
        NutList *nuts = inv->Items(kFoodNut);
        for (int i = 0; i < drop; i++)
            ((GoodiePlace *)place)->Goodies()->Drop(nuts->At(i), kFoodNut, *Body()->GetMatrix());
        for (; drop > 0; drop--)
            inv->Remove(kFoodNut, 0);
    }
    count = inv->Count(kSpitNut);
    if (count) {
        int drop = count > 1 ? (int)(count * gSpitNutSprayPercent) : 1;
        NutList *nuts = inv->Items(kSpitNut);
        for (int i = 0; i < drop; i++)
            ((GoodiePlace *)place)->Goodies()->Drop(nuts->At(i), kSpitNut, *Body()->GetMatrix());
        for (; drop > 0; drop--)
            inv->Remove(kSpitNut, 0);
    }
}
