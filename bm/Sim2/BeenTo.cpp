// SqueezilRegionSoul::BeenTo (SQZ.EXE 0x437f40, ret 4): the pair (this region, place) is in the inventory's "PlacesBeenTo".
struct IntListB { virtual int Get(int i); };
struct InventoryB {
    int Count(const char *kind);                // 0x416300
    IntListB *Items(const char *kind);          // 0x4163d0
};
struct LentB { InventoryB *Inventory(); };      // 0x448990
struct LandB {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual LentB *Lend(int id);                // slot 10 (+0x28)
};
LandB *GetLand();                               // 0x420680
struct RegionB { int GetRegion(); };            // 0x418160

struct RegionSoulB {
    char pad[0x88];
    RegionB *region;                            // 0x88
    int BeenTo(int place);
};

int RegionSoulB::BeenTo(int place)
{
    int i = 0;
    int count = GetLand()->Lend(0)->Inventory()->Count("PlacesBeenTo");
    int result = 0;
    if (0 < count) {
        IntListB *done = GetLand()->Lend(0)->Inventory()->Items("PlacesBeenTo");
        if (0 < count) {
            for (; count > i; i += 2) {
                if (region->GetRegion() == done->Get(i) && done->Get(i + 1) == place) {
                    result = 1;
                    break;
                }
            }
        }
    }
    return result;
}
