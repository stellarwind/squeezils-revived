// PlaceSimulation::Close (virtual slot 7, SQZ.EXE 0x439a00): drain the pending work, shut the parts down and fall back to the location base.
int Pending();                                           // 0x44aae0 (cdecl)
struct PlaceGrid { void Stop(); void Free(); };          // 0x40e160 / 0x40ec90
struct PlaceLevel { void Shutdown(); };                  // 0x44b2e0
struct LocationBase { void Close(); };                   // 0x413060
void ReleaseGlobalA();                                   // 0x422b80 (cdecl)
void ReleaseGlobalB();                                   // 0x42d010 (cdecl)
struct PlaceSimulation {
    char pad[0x38];
    PlaceLevel *level;                                   // 0x38
    PlaceGrid *grid;                                     // 0x3c

    void Close();                                        // slot 7
};

void PlaceSimulation::Close()
{
    while (Pending() > 0)
        ;
    grid->Stop();
    grid->Free();
    PlaceLevel *l = level;
    if (l) {
        l->Shutdown();
        delete l;
    }
    ReleaseGlobalA();
    ReleaseGlobalB();
    ((LocationBase *)this)->Close();
}
