// RegionSimulation::LocationState (SQZ.EXE 0x418590, ret 8): 0 finished, 1 finished and next to the player, 2 open, 3 open and next to the player.
struct IntListR { virtual int Get(int i); int count; };    // slot 0; count at +4
struct InventoryR {
    int Count(const char *kind);                // 0x416300
    IntListR *Items(const char *kind);          // 0x4163d0
};
struct LentR { InventoryR *Inventory(); };      // 0x448990
struct LandR {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual LentR *Lend(int id);                // slot 10 (+0x28)
};
LandR *GetLand();                               // 0x420680
struct PlaceR {
    char pad[0x10];
    IntListR *links;                            // 0x10
};

struct RegionSimR {
    char pad[0x24];
    int region;                                 // 0x24
    int location;                               // 0x28
    int LocationState(PlaceR *place, int id);
};

int RegionSimR::LocationState(PlaceR *place, int id)
{
    int finished = 0;
    LentR *possessions = GetLand()->Lend(0);
    IntListR *done = 0;
    int count = possessions->Inventory()->Count("PlacesCompleted");
    if (count > 0)
        done = possessions->Inventory()->Items("PlacesCompleted");
    for (int i = 0; count > i && !finished; i += 2) {
        if (done->Get(i) == region && done->Get(i + 1) == id)
            finished = 1;
    }
    int nextTo = 0;
    int j = 0;
    for (; j < place->links->count; j++)
        if (place->links->Get(j) == location)
            nextTo = 1;
    if (!finished)
        return nextTo ? 3 : 2;
    return nextTo ? 1 : 0;
}
