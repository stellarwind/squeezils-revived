// SqueezilRegionSoul::LinkOpen (SQZ.EXE 0x437e40, ret 8): whether the player has earned the way over a portal.
struct Destinations { virtual int At(int i); };    // slot 0
struct PlaceDescriptor {
    char pad[0xc];
    Destinations *destinations;     // 0x0c
    char pad2[0x2c];
    int kind;                       // 0x3c
};
struct Inventory {
    char pad[0x54];
    int relic[8];                   // 0x54
    int AllRelics();                // 0x4165b0
    int KeysFound(int region);      // 0x416580
};
struct Owner { Inventory *GetInventory(); };   // 0x448990
struct Lender { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
                virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
                virtual Owner *Lend(int i); };   // slot 10 (+0x28)
Lender *GetLand();                  // 0x420680
struct RegionRef { int GetRegion(); };         // 0x418160

struct SqueezilRegionSoul {
    char pad[0x88];
    RegionRef *region;              // 0x88
    int LinkOpen(PlaceDescriptor *from, PlaceDescriptor *here);
};

int SqueezilRegionSoul::LinkOpen(PlaceDescriptor *, PlaceDescriptor *here)
{
    if (here->kind != 2)
        return 0;
    Inventory *inventory = GetLand()->Lend(0)->GetInventory();
    if (region->GetRegion() == 1) {
        if (here->destinations->At(0) == 6)
            return inventory->AllRelics() != 0;
        return 1;
    }
    if (region->GetRegion() != 6 && region->GetRegion() != 7)
        return inventory->relic[region->GetRegion()] != 0;
    if (region->GetRegion() == 6) {
        if (here->destinations->At(0) != 7)
            return 0;
        return inventory->KeysFound(region->GetRegion()) != 0;
    }
    return 0;
}
