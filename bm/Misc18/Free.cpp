// SQZ.EXE 0x405490: delete every object in a 10x10 grid (rows of 16 slots) and the two extra objects at +0x2a0/+0x2a4.
struct Obj { ~Obj(); };                 // 0x473890
struct Holder2 {
    Obj *grid[10][16];                  // +0
    char pad[0x20];
    Obj *extraA;                        // +0x2a0
    Obj *extraB;                        // +0x2a4
    void FreeAll();
};
void Holder2::FreeAll()
{
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            delete grid[i][j];
    delete extraA;
    delete extraB;
}
