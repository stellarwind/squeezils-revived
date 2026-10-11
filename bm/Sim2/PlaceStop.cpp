// PlaceSimulation::Stop (virtual slot 3, SQZ.EXE 0x439b00): leave the place, or shut the parts down when it was never started.
struct PlaceGrid { void Reset(); };                      // 0x4106e0
struct PlaceBody { void Pause(); void Release(); };      // 0x41f7f0 / 0x41f750
struct PlaceLevel { void Close(); };                     // 0x44e0f0
struct LocationBase { void Teardown(); };                // 0x413f80
struct PlaceSimulation {
    char pad[0x34];
    int started;                                         // 0x34
    PlaceLevel *level;                                   // 0x38
    PlaceGrid *grid;                                     // 0x3c
    PlaceBody *body;                                     // 0x40
    void Stop();                                         // slot 3
};

void PlaceSimulation::Stop()
{
    if (started) {
        body->Pause();
        return;
    }
    ((LocationBase *)this)->Teardown();
    level->Close();
    body->Release();
    grid->Reset();
}
