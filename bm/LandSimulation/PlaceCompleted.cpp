// LandSimulation::PlaceCompleted (SQZ.EXE 0x41e9b0, ret 8): the player has finished that place (the pairs of "PlacesCompleted" in the inventory).
struct IntListC { virtual int Get(int i); };    // slot 0
struct InventoryC {
    int Count(const char *kind);                // 0x416300
    IntListC *Items(const char *kind);          // 0x4163d0
};
struct Lent { InventoryC *Inventory(); };       // 0x448990
struct LandC {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual Lent *Lend(int id);                 // slot 10 (+0x28)
    int PlaceCompleted(int region, int place);
};
extern const char kPlacesCompleted[];           // 0x4ba668

int LandC::PlaceCompleted(int region, int place)
{
    int result = 0;
    int count = Lend(0)->Inventory()->Count("PlacesCompleted");
    if (count > 0) {
        IntListC *done = Lend(0)->Inventory()->Items("PlacesCompleted");
        if (count > 0) {
            for (int i = 0; i < count; i += 2) {
                if (done->Get(i) == region && done->Get(i + 1) == place) {
                    result = 1;
                    break;
                }
            }
        }
    }
    return result;
}
