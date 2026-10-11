// LocationSimulation slot 2 (SQZ.EXE 0x4130b0): free the owned list and detach from the global tables.
struct LocList { void Clear(); };                        // 0x412d20
struct LocGlobalA { void Detach(); };                    // 0x435b30
LocGlobalA *GetGlobalA();                                // 0x4206a0 (cdecl)
void Finish();                                           // 0x443260 (cdecl)
struct LocationSimulation {
    int pad;
    LocList *list;                                       // 0x04
    void Free();                                         // slot 2
};

void LocationSimulation::Free()
{
    LocList *l = list;
    if (l) {
        l->Clear();
        delete l;
    }
    GetGlobalA()->Detach();
    Finish();
}
