// LocationSimulation slot 1 (SQZ.EXE 0x412f90): make the creature table (ids start at 10), then drain the pending input.
struct TableBase { TableBase(); virtual ~TableBase(); char pad[0x28]; };   // ctor 0x466590, dtor 0x4666b0
struct BufObj { virtual void bo0(); };
struct TableMid : TableBase, BufObj { TableMid() {} virtual ~TableMid(); };                  // dtor 0x414330
struct CreatureTable : TableMid { CreatureTable() {} virtual ~CreatureTable(); virtual void bo0(); };
struct InputQueue { void Flush(); };                     // 0x43c970
InputQueue *GetInput();                                  // 0x4206b0 (cdecl)
struct LocationSimulation {
    int pad;
    CreatureTable *table;                                // 0x04
    int lastId;                                          // 0x08
    void Init();                                         // slot 1
};

void LocationSimulation::Init()
{
    CreatureTable *t = new CreatureTable;
    lastId = 10;
    table = t;
    GetInput()->Flush();
}
