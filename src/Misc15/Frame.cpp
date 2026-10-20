// The per-frame update (SQZ.EXE 0x420760): timed section running the world, the sim and the scene.
struct FrameTimer { void Start(); void Stop(); };          // 0x41ec20 / 0x41ec30
struct FrameSim { virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void Update(); };   // slot 4 (+0x10)
struct FrameScene { void Present(); };                     // 0x4362a0
extern FrameTimer *gFrameTimer;                            // 0x4badc0
extern FrameSim *gFrameSim;                                // 0x4badbc
FrameScene *GetFrameScene();                               // 0x4206a0
void UpdateInput();                                        // 0x423050
void UpdateCreatures();                                    // 0x42d250

void RunFrame()
{
    gFrameTimer->Start();
    UpdateInput();
    UpdateCreatures();
    if (gFrameSim)
        gFrameSim->Update();
    GetFrameScene()->Present();
    gFrameTimer->Stop();
}
