// PlaceSimulation constructor (SQZ.EXE 0x438850): the location base, then the vtable and the three ids.
struct LocationSimulation { char pad[0x20]; LocationSimulation(); virtual ~LocationSimulation(); };   // ctor 0x412e80, dtor 0x412f20
struct PlaceSimulation : LocationSimulation {
    int region, place, location;                         // 0x24 0x28 0x2c
    PlaceSimulation(int r, int p, int l);
    virtual ~PlaceSimulation();
};

PlaceSimulation::PlaceSimulation(int r, int p, int l)
{
    region = r;
    place = p;
    location = l;
}
