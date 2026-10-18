inline void *operator new(unsigned, void *p) { return p; }
// LandSimulation constructor (SQZ.EXE 0x41e310): Simulation and BufferObject bases built inline (vtables stored twice), then the own fields.
class SimulationB {
public:
    virtual ~SimulationB();                  // 0x414270
    virtual void s1();
};
class BufferObjectB {
public:
    BufferObjectB() {}
    virtual void SaveSlot0();
    virtual void SaveSlot1();
    virtual void SaveSlot2();
    virtual void SaveSlot3();
};
class LandSim : public SimulationB, public BufferObjectB {
public:
    LandSim();
    virtual ~LandSim();
    virtual void s1();
    void *table;                             // 0x08
    void *lentIds;                           // 0x0c
    char **names;                            // 0x10
};

LandSim::LandSim()
{
    table = 0;
    lentIds = 0;
}

LandSim *MakeLandSim(LandSim *p)
{
    return new (p) LandSim;
}
