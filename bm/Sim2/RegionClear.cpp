// 6 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
// RegionSimulation slot 2 (SQZ.EXE 0x417600): drain the audio, stop the sound and music, free every place record, fall back to the base.
int Pending();                                           // 0x44aae0 (cdecl)
void UnloadSounds();                                     // 0x4499e0 (cdecl)
void StopMidi();                                         // 0x45aa70 (cdecl)
extern int g_midi;                                       // 0x4bf458
void ReleaseA();                                         // 0x422b80 (cdecl)
void ReleaseB();                                         // 0x422b00 (cdecl)
void ReleaseC();                                         // 0x448330 (cdecl)
void ReleaseD();                                         // 0x42d010 (cdecl)
struct PlaceRecord { void Destruct(); };                 // 0x416d40
struct LocationBase { void ClearCreatures(); void Free(); };   // 0x413060 / 0x4130b0
struct RegionSimulation {
    char pad[0x38];
    int count;                                           // 0x38
    char pad2[0x54 - 0x3c];
    PlaceRecord **places;                                // 0x54
    void Clear();                                        // slot 2
};

void RegionSimulation::Clear()
{
    while (Pending() > 0)
        ;
    UnloadSounds();
    if (g_midi)
        StopMidi();
    ReleaseA();
    ReleaseB();
    ReleaseC();
    ReleaseD();
    for (int i = 0; i < count; i++) {
        PlaceRecord *p = places[i];
        if (p) {
            p->Destruct();
            delete p;
        }
    }
    ((LocationBase *)this)->ClearCreatures();
    ((LocationBase *)this)->Free();
}
