// 13 spare declarations (symbol count, see padscan.sh)
void PadDecl1();
void PadDecl2();
void PadDecl3();
void PadDecl4();
void PadDecl5();
void PadDecl6();
void PadDecl7();
void PadDecl8();
void PadDecl9();
void PadDecl10();
void PadDecl11();
void PadDecl12();
void PadDecl13();
// 0x43de80 / 0x43eb30 (ret 4): read binding i's field as the registry would leave it: reload from the registry, but keep the
// in-memory value when the stored binding is of kind 2. The two functions read adjacent fields.
#include <string.h>
struct Bind {
    int w;              // 0x00
    int a;              // 0x04
    int kind;           // 0x08
    char rest[0x44 - 0x0c];
};
struct Block { char pad[0xc]; Bind e[1]; };           // mirror of this+0xc..
struct Bindings43b {
    char pad[0x0c];
    Block blk;
    int Load();         // 0x43ec70
    int PeekA(int i);   // 0x43de80
    int PeekW(int i);   // 0x43eb30
};
int Bindings43b::PeekA(int i)
{
    Block *backup = (Block *)new char[0xd8c];
    memcpy(backup, &blk, 0xd8c);
    if (!Load()) {
        delete backup;
        return 0;
    }
    int v;
    if (blk.e[i].kind != 2)
        v = blk.e[i].a;
    else
        v = backup->e[i].a;
    memcpy(&blk, backup, 0xd8c);
    delete backup;
    return v;
}
int Bindings43b::PeekW(int i)
{
    Block *backup = (Block *)new char[0xd8c];
    memcpy(backup, &blk, 0xd8c);
    if (!Load()) {
        delete backup;
        return 0;
    }
    int v;
    if (blk.e[i].kind != 2)
        v = blk.e[i].w;
    else
        v = backup->e[i].w;
    memcpy(&blk, backup, 0xd8c);
    delete backup;
    return v;
}
