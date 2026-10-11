// PlaceSimulation::Step (virtual slot 8, SQZ.EXE 0x439ae0): step the grid, then run the location base.
struct PlaceGrid { void Step(); };                       // 0x410330
struct LocationBase { void Run(); };                     // 0x413280
struct PlaceSimulation {
    char pad[0x3c];
    PlaceGrid *grid;                                     // 0x3c
    void Step();                                         // slot 8
};

void PlaceSimulation::Step()
{
    grid->Step();
    ((LocationBase *)this)->Run();
}
