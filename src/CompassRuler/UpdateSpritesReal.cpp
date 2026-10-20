// CompassRuler::UpdateSpritesReal (SQZ.EXE 0x414af0): choose each sector's picture, the first or the second once the place is done.
struct Bitmap;
struct Inv { char pad[0x58]; int visited[8]; };
struct InvOwner { Inv *Inventory(); };      // 0x448990
struct SoulX { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
               virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
               virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17();
               virtual InvOwner *Owner(); };   // slot 18 (+0x48)
struct SimX { SoulX *GetSoul(int id); };    // 0x414080
SimX *GetCurrentSimulation();               // 0x420690

struct CompassRulerR {
    Bitmap *picture[12];                    // 0x00
    int x, y;                               // 0x30
    Bitmap *sprite[8];                      // 0x38
    int sector, sectorCount;                // 0x58
    float fullTurn;                         // 0x60
    int pineRegion, desertRegion, coastRegion, jungleRegion;   // 0x64
    float quarter, eighth;                  // 0x74
    void UpdateSpritesReal();
};

void CompassRulerR::UpdateSpritesReal()
{
    if (!GetCurrentSimulation()) {
        sprite[0] = picture[6];
        sprite[1] = picture[10];
        sprite[2] = picture[4];
        sprite[3] = picture[10];
        sprite[4] = picture[2];
        sprite[5] = picture[10];
        sprite[6] = picture[8];
        sprite[7] = picture[10];
    } else {
        SoulX *soul = GetCurrentSimulation()->GetSoul(0);
        if (!soul)
            return;
        InvOwner *owner = soul->Owner();
        if (!owner)
            return;
        sprite[0] = owner->Inventory()->visited[pineRegion] == 0 ? picture[6] : picture[7];
        sprite[1] = picture[10];
        sprite[2] = owner->Inventory()->visited[desertRegion] == 0 ? picture[4] : picture[5];
        sprite[3] = picture[10];
        sprite[4] = owner->Inventory()->visited[coastRegion] == 0 ? picture[2] : picture[3];
        sprite[5] = picture[10];
        sprite[6] = owner->Inventory()->visited[jungleRegion] == 0 ? picture[8] : picture[9];
        sprite[7] = picture[10];
    }
    sector = 3;
    sectorCount = 8;
    fullTurn = 6.283185f;
    quarter = 0.785398f;
    eighth = 0.392699f;
}
