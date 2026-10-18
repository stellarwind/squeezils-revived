// CreatureTable::~CreatureTable (SQZ.EXE 0x412d20): delete the creature records, then the table part (0x414330).
struct TableBase { TableBase(); virtual ~TableBase(); int pad4; int count; char pad[0x18]; int **items; int tail; };   // dtor 0x4666b0
struct BufObj { virtual void bo0(); };
struct TableMid : TableBase, BufObj { TableMid() {} virtual ~TableMid(); };                  // dtor 0x414330
struct CreatureTable : TableMid { CreatureTable() {} virtual ~CreatureTable(); virtual void bo0(); };

CreatureTable::~CreatureTable()
{
    int i = 0;
    for (;;) {
        if (i >= count)
            break;
        delete items[i];
        i++;
    }
}
void CreatureTable::bo0() {}
