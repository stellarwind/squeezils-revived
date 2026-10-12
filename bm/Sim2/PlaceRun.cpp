// PlaceSimulation slot 4 (SQZ.EXE 0x439b40): the base's RunActors, as a tail jump.
struct LocationSimulation { void RunActors(); };         // 0x414010
struct PlaceSimulation : LocationSimulation {
    void Run();                                          // slot 4
};

void PlaceSimulation::Run()
{
    RunActors();
}
