// PlaceSimulation slot 2 (SQZ.EXE 0x439a50): the whole shutdown, then fall back to the location base.
struct PlaceGrid { void Stop(); void Free(); void Destroy(); };   // 0x40ede0 / 0x40e160 / 0x40d640
int Pending();                                       // 0x44aae0 (cdecl)
void ReleaseA(int);                                      // 0x422bc0 (cdecl)
void ReleaseB();                                         // 0x422b00 (cdecl)
void ReleaseC();                                         // 0x448330 (cdecl)
void UnloadSounds();                                     // 0x4499e0 (cdecl)
void StopMidi();                                         // 0x45aa70 (cdecl)
extern int g_midi;                                       // 0x4bf458
struct LocationBase { void Free(); };                    // 0x4130b0
struct PlaceSimulation {
    char pad[0x38];
    PlaceGrid *grid;                                     // 0x3c
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5(); virtual void v6();
    virtual void ClearCreatures();                       // slot 7 (+0x1c)

    void Clear();                                        // slot 2
};

void PlaceSimulation::Clear()
{
    ClearCreatures();
    while (Pending() > 0)
        ;
    UnloadSounds();
    if (g_midi)
        StopMidi();
    grid->Stop();
    ReleaseA(0);
    ReleaseB();
    ReleaseC();
    grid->Free();
    ReleaseB();
    grid->Free();
    PlaceGrid *g = grid;
    if (g) {
        g->Destroy();
        delete g;
    }
    ((LocationBase *)this)->Free();
}
