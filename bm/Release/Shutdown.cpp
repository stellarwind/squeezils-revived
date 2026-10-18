// Game shutdown (SQZ.EXE 0x4205a0): tear down the simulation, land, and every global subsystem.
struct Sim {
    virtual ~Sim();
    virtual void v1();
    virtual void Close();           // slot 2 (+8)
};
struct Application { ~Application(); };   // 0x43a730
struct World { ~World(); };               // 0x4358e0
struct GameClock { ~GameClock(); };       // 0x41ebe0
struct Panel { ~Panel(); };               // 0x42d880

extern Sim *g_currentSimulation;    // 0x4badbc
extern Sim *g_land;                 // 0x4badb8
extern Application *g_application;  // 0x4badac
extern World *g_world;              // 0x4bada8
extern GameClock *g_gameClock;      // 0x4badc0
extern Panel *g_panel;              // 0x4badb0

void ShutdownA();                   // 0x429dc0 (cdecl)
void ShutdownB();                   // 0x448330
void ReleaseB();                    // 0x422b00
void ShutdownC();                   // 0x4580a0
void ShutdownD();                   // 0x421d80

void Shutdown()
{
    if (g_currentSimulation) {
        g_currentSimulation->Close();
        delete g_currentSimulation;
    }
    if (g_land) {
        g_land->Close();
        delete g_land;
    }
    ShutdownA();
    if (g_application)
        delete g_application;
    if (g_world)
        delete g_world;
    if (g_gameClock)
        delete g_gameClock;
    ShutdownB();
    ReleaseB();
    if (g_panel)
        delete g_panel;
    ShutdownC();
    ShutdownD();
}
