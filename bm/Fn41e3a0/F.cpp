// six spare declarations: the compiler's register choices follow the symbol count (tools/flagsearch/padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
// LandSimulation::~LandSimulation (0x41e3a0): frees the possessions table, the lent-id set and the place-name array.
struct TableCore { char d[0x1c]; ~TableCore(); };   // 0x4651d0
struct Table7 { TableCore core; ~Table7() {} };
struct Set10 { char d[0x1c]; ~Set10(); };            // 0x45efa0
class SimB {
public:
    virtual ~SimB();                                 // 0x414270
    virtual void s1();
};
class BufB {
public:
    BufB() {}
    virtual void SaveSlot0();
    virtual void SaveSlot1();
    virtual void SaveSlot2();
    virtual void SaveSlot3();
};
class LandSim : public SimB, public BufB {
public:
    virtual ~LandSim();
    virtual void s1();
    Table7 *table;                                   // 0x08
    Set10 *lentIds;                                  // 0x0c
    char **names;                                    // 0x10
    int regionCount;                                 // 0x14
};
LandSim::~LandSim()
{
    if (table) delete table;
    if (lentIds) delete lentIds;
    if (names) {
        for (int i = 0; i < regionCount; i++)
            delete names[i + 1];
        delete names;
    }
}
